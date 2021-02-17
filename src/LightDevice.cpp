#include "LightDevice.h"

Device::Device(String name) {
  this->name = name;
  this->clientsCounter = 0;

  this->serviceList = new ServiceNode();
  this->serviceList->service = nullptr;
  this->serviceList->next = nullptr;
}

DynamicJsonDocument Device::prepareMessage(int capacity, String type) {
  DynamicJsonDocument doc(capacity);
  doc["messageType"] = type;
  return doc;
}

void Device::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                            size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected\n", num);
    clients[num].setDisconnected();
    break;
  case WStype_CONNECTED: {
    IPAddress ip = this->webSocket->remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0],
                  ip[1], ip[2], ip[3], payload);
    if (!this->isFreeSpaceForNewClient()) {
      clients[num].setConnected();
      this->sender->send("{ \"messageType\": \"noSpace\" }", clients[num]);
      this->webSocket->disconnect(num);
      clients[num].setDisconnected();
    } else {
      clients[num].setId(String(clientsCounter++));
      clients[num].setConnected();
    }
  } break;
  case WStype_TEXT: {
    Serial.printf("[%u] get Text: %s\n", num, payload);
    String message = (char *)payload;
    this->interpretMessage(this->clients[num], message);
  } break;
  case WStype_FRAGMENT_TEXT_START:
    this->fragmentBuffer[num] = (char *)payload;
    Serial.printf("[%u] get start start of Textfragment: %s\n", num, payload);
    break;
  case WStype_FRAGMENT:
    this->fragmentBuffer[num] += (char *)payload;
    Serial.printf("[%u] get Textfragment : %s\n", num, payload);
    break;
  case WStype_FRAGMENT_FIN:
    this->fragmentBuffer[num] += (char *)payload;
    Serial.printf("[%u] get end of Textfragment: %s\n", num, payload);
    Serial.printf("[%u] full frame: %s\n", num, fragmentBuffer[num].c_str());
    this->interpretMessage(this->clients[num], fragmentBuffer[num]);
    break;
  }
}

void Device::interpretMessage(HClient &client, String message) {
  String restMessage = message;
  String jsonMessage = "";

  while (restMessage.length() > 0) {
    bool foundMessage = false;
    int jsonObjectsNum = 0;
    for (int i = 0; i < restMessage.length(); i++) {
      if (restMessage[i] == '{') {
        jsonObjectsNum++;
      } else if (restMessage[i] == '}') {
        jsonObjectsNum--;
        if (jsonObjectsNum == 0) {
          jsonMessage = restMessage.substring(0, i + 1);
          restMessage = restMessage.substring(i + 1);

          Serial.printf("Parsing JSON: %s\n", jsonMessage.c_str());

          DynamicJsonDocument doc(INCOMING_JSON_CAPACITY);
          DeserializationError deserializationError =
              deserializeJson(doc, jsonMessage);
          if (deserializationError.code() == deserializationError.Ok) {
            this->interpretMessage(client, doc);
            foundMessage = true;
          } else {
            Serial.printf("Error parsing JSON: %s\n",
                          deserializationError.c_str());
          }

          break;
        }
      }
    }
    // could not find json message :(
    if (!foundMessage) {
      Serial.println("Could not find JSON message!");
      return;
    }
  }
}

void Device::interpretMessage(HClient &client, DynamicJsonDocument &json) {
  String messageType = json["messageType"];

  if (messageType.equals("authenticate")) {
    if (!json.containsKey("data")) {
      return;
    }
    JsonObject data = json["data"];
    if (!data.containsKey("password")) {
      return;
    }
    String password = data["password"];
    if (password.equals(this->devicePassword)) {
      client.setAuthenticated(true);
      sendSimpleMessage(client, "authenticationSuccess");
    } else {
      sendSimpleMessage(client, "authenticationFail");
    }
  } else if (messageType.equals("keepalive")) {
    client.keepalive();
  } else if (messageType.equals("ping")) {
    sendSimpleMessage(client, "pong");
  } else if (messageType.equals("describe")) {
    if (!client.isAuthenticated()) {
      sendSimpleMessage(client, "authenticationRequired");
      return;
    }

    DynamicJsonDocument doc = this->prepareMessage(DESCRIBE_JSON_SIZE, "serviceDescriptions");
    JsonObject data = doc.createNestedObject("data");

    ServiceNode *serviceNode = this->serviceList;
    while (serviceNode->next != nullptr) {
      JsonObject service = data.createNestedObject(serviceNode->service->getId());
      serviceNode->service->createJSONDescription(service);
      serviceNode = serviceNode->next;
    }

    String output;
    serializeJson(doc, output);
    this->sender->send(output, client);

  } else if (messageType.equals("serviceInteraction")) {
    if (!client.isAuthenticated()) {
      sendSimpleMessage(client, "authenticationRequired");
      return;
    }
    if (!json.containsKey("data")) {
      return;
    }
    JsonObject data = json["data"];
    if (!data.containsKey("serviceId")) {
      return;
    }
    if (!data.containsKey("data")) {
      return;
    }
    String serviceId = data["serviceId"];
    JsonObject messageToService = data["data"];
    Service *service = findServiceWithId(serviceId);
    if (service != nullptr) {
      service->interpretMessage(client, sender, messageToService);
    }
  }
}

void Device::setup(String ssid, String password, int port) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  WiFi.setAutoReconnect(true);
  Serial.print("IP address:\t");
  Serial.print(WiFi.localIP());
  Serial.print("\tPort:\t");
  Serial.println(port);
  this->port = port;
}

void Device::start() {
  this->webSocket = new WebSocketsServer(this->port);
  this->webSocket->begin();

  auto eventFunction = [&](uint8_t num, WStype_t type, uint8_t *payload,
                           size_t length) {
    this->webSocketEvent(num, type, payload, length);
  };
  this->webSocket->onEvent(eventFunction);

  this->sender = new Sender(this->webSocket, clients);
}

Service *Device::findServiceWithId(String id) {
  ServiceNode *serviceNode = this->serviceList;
  while (serviceNode->next != nullptr) {
    if (serviceNode->service->getId().equals(id)) {
      return serviceNode->service;
    }
    serviceNode = serviceNode->next;
  }
  return nullptr;
}

void Device::addService(Service *service) {
  service->setDevice(this);
  ServiceNode *newNode = new ServiceNode();
  newNode->service = service;
  newNode->next = this->serviceList;
  this->serviceList = newNode;
}

void Device::sendSimpleMessage(HClient &client, String type) {
  DynamicJsonDocument doc = this->prepareMessage(SMALL_MESSAGE_JSON_SIZE, type);
  String output;
  serializeJson(doc, output);
  this->sender->send(output, client);
}

bool Device::isFreeSpaceForNewClient() {
  int clientsConnected = 0;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (this->clients[i].isConnected()) {
      clientsConnected++;
    }
  }
  Serial.print("Connected: ");
  Serial.println(clientsConnected);
  return clientsConnected < (MAX_CLIENTS - 1);
}

bool Device::isMessageProper(DynamicJsonDocument &json) {
  return json.containsKey("messageType");
}

void Device::update() {
  this->webSocket->loop();
  ServiceNode *serviceNode = this->serviceList;
  while (serviceNode->next != nullptr) {
    serviceNode->service->update(sender);
    serviceNode = serviceNode->next;
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (this->clients[i].isConnected()) {
      if (this->clients[i].isKeepaliveTimeout()) {
        sendSimpleMessage(this->clients[i], "keepaliveTimeout");
        this->webSocket->disconnect(this->clients[i].getSocketId());
      }
    }
  }
}
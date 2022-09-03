#include "LightDevice.h"

Device::Device(char const *name, int port)
{
  this->name = name;
  this->port = port;
  this->clients = new HClient*[LIGHTHOUSE_CLIENT_MAX];
  for (int i = 0; i < LIGHTHOUSE_CLIENT_MAX; i++) {
    this->clients[i] = new HClient(i);
  }

  this->logsMode = Logs::SIMPLE;
  this->clientsCounter = 0;

  this->serviceList = new ServiceNode();
  this->serviceList->service = nullptr;
  this->serviceList->next = nullptr;

  this->isOTAEnabled = false;
  this->isWifiSetupEnabled = false;
  this->isUDPActive = false;
  this->wifiStatusNotifier = nullptr;
}

void Device::setUDPSupport(int port)
{
  this->udp = new WiFiUDP();
  this->udpPort = port;
  this->isUDPActive = true;
}

void Device::setWifiStatusNotifier(void (*wifiStatusNotifier)(WiFiStatus status), int wifiStatusConnectingPulseTime)
{
  this->wifiStatusNotifier = wifiStatusNotifier;
  this->wifiStatusConnectingPulseTime = wifiStatusConnectingPulseTime;
}

DynamicJsonDocument Device::prepareMessage(int capacity, String type)
{
  DynamicJsonDocument doc(capacity);
  doc["messageType"] = type;
  return doc;
}

void Device::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                            size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    logf(Logs::DETAILED, "[%u] Disconnected\n", num);
    clients[num]->setDisconnected();
    clients[num]->setEmpty(true);
    break;
  case WStype_CONNECTED:
  {
    IPAddress ip = this->webSocket->remoteIP(num);
    logf(Logs::DETAILED, "[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0],
         ip[1], ip[2], ip[3], payload);
    if (!this->isFreeSpaceForNewClient())
    {
      clients[num]->setConnected();
      clients[num]->setEmpty(false);
      this->mainSender->send("{ \"messageType\": \"noSpace\" }", clients[num]);
      clients[num]->setDisconnected();
      this->webSocket->disconnect(num);
    }
    else
    {
      clients[num]->setId(String(clientsCounter++));
      clients[num]->setConnected();
      clients[num]->setEmpty(false);
    }
  }
  break;
  case WStype_TEXT:
  {
    if (!clients[num]->isConnected()) {
      break;
    }
    logf(Logs::DETAILED, "[%u] get Text: %s\n", num, payload);
    String message = (char *)payload;
    this->interpretMessage(this->clients[num], this->mainSender, message);
  }
  break;
  case WStype_FRAGMENT_TEXT_START:
    if (!clients[num]->isConnected()) {
      break;
    }
    this->fragmentBuffer[num] = (char *)payload;
    logf(Logs::DETAILED, "[%u] get start start of Textfragment: %s\n", num, payload);
    break;
  case WStype_FRAGMENT:
    if (!clients[num]->isConnected()) {
      break;
    }
    this->fragmentBuffer[num] += (char *)payload;
    logf(Logs::DETAILED, "[%u] get Textfragment : %s\n", num, payload);
    break;
  case WStype_FRAGMENT_FIN:
    if (!clients[num]->isConnected()) {
      break;
    }
    this->fragmentBuffer[num] += (char *)payload;
    logf(Logs::DETAILED, "[%u] get end of Textfragment: %s\n", num, payload);
    logf(Logs::DETAILED, "[%u] full frame: %s\n", num, fragmentBuffer[num].c_str());
    this->interpretMessage(this->clients[num], this->mainSender, fragmentBuffer[num]);
    break;
  }
}

void Device::interpretMessage(HClient *client, Sender *sender, String message)
{
  String restMessage = message;
  String jsonMessage = "";

  while (restMessage.length() > 0)
  {
    bool foundMessage = false;
    int jsonObjectsNum = 0;
    for (int i = 0; i < restMessage.length(); i++)
    {
      if (restMessage[i] == '{')
      {
        jsonObjectsNum++;
      }
      else if (restMessage[i] == '}')
      {
        jsonObjectsNum--;
        if (jsonObjectsNum == 0)
        {
          jsonMessage = restMessage.substring(0, i + 1);
          restMessage = restMessage.substring(i + 1);

          logf(Logs::DETAILED, "Parsing JSON: %s\n", jsonMessage.c_str());

          DynamicJsonDocument doc(INCOMING_JSON_CAPACITY);
          DeserializationError deserializationError =
              deserializeJson(doc, jsonMessage);
          if (deserializationError.code() == deserializationError.Ok)
          {
            this->interpretMessage(client, sender, doc);
            foundMessage = true;
          }
          else
          {
            logf(Logs::DETAILED, "Error parsing JSON: %s\n",
                 deserializationError.c_str());
          }

          break;
        }
      }
    }
    // could not find json message :(
    if (!foundMessage)
    {
      logf(Logs::DETAILED, "Could not find JSON message!");
      return;
    }
  }
}

void Device::interpretMessage(HClient *client, Sender *sender, DynamicJsonDocument &json)
{
  String messageType = json["messageType"];

  if (messageType.equals("authenticate"))
  {
    if (!json.containsKey("data"))
    {
      return;
    }
    JsonObject data = json["data"];
    if (!data.containsKey("password"))
    {
      return;
    }
    String password = data["password"];
    if (password.equals(this->devicePassword))
    {
      client->setAuthenticated(true);
      sendSimpleMessage(sender, client, "authenticationSuccess");
    }
    else
    {
      sendSimpleMessage(sender, client, "authenticationFail");
    }
  }
  else if (messageType.equals("keepalive"))
  {
    client->keepalive();
  }
  else if (messageType.equals("ping"))
  {
    sendSimpleMessage(sender, client, "pong");
  }
  else if (messageType.equals("describe"))
  {
    if (!client->isAuthenticated())
    {
      sendSimpleMessage(sender, client, "authenticationRequired");
      return;
    }

    DynamicJsonDocument doc = this->prepareMessage(DESCRIBE_JSON_SIZE, "serviceDescriptions");
    JsonObject data = doc.createNestedObject("data");

    ServiceNode *serviceNode = this->serviceList;
    while (serviceNode->next != nullptr)
    {
      JsonObject service = data.createNestedObject(serviceNode->service->getId());
      serviceNode->service->createJSONDescription(service);
      serviceNode = serviceNode->next;
    }

    String output;
    serializeJson(doc, output);
    sender->send(output, client);
  }
  else if (messageType.equals("serviceInteraction"))
  {
    if (!client->isAuthenticated())
    {
      sendSimpleMessage(sender, client, "authenticationRequired");
      return;
    }
    if (!json.containsKey("data"))
    {
      return;
    }
    JsonObject data = json["data"];
    if (!data.containsKey("serviceId"))
    {
      return;
    }
    if (!data.containsKey("data"))
    {
      return;
    }
    String serviceId = data["serviceId"];
    JsonObject messageToService = data["data"];
    if (serviceId.equals("*"))
    {
      ServiceNode *serviceNode = this->serviceList;
      while (serviceNode->next != nullptr)
      {
        serviceNode->service->interpretMessage(client, sender, messageToService);
        serviceNode = serviceNode->next;
      }
    }
    else
    {
      Service *service = findServiceWithId(serviceId);
      if (service != nullptr)
      {
        service->interpretMessage(client, sender, messageToService);
      }
    }
  }
  else if (messageType.equals("setupLogs"))
  {
    if (!client->isAuthenticated())
    {
      sendSimpleMessage(sender, client, "authenticationRequired");
      return;
    }
    if (!json.containsKey("data"))
    {
      return;
    }
    JsonObject data = json["data"];
    if (!data.containsKey("enabled"))
    {
      return;
    }
    bool enabled = data["enabled"];
    client->setLogEnabled(enabled);
  }
}

void Device::ensureHasWifi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    log(Logs::SIMPLE, "Connecting to WiFi...");
    WiFi.disconnect();
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      if (counter >= 4)
      {
        if (wifiStatusNotifier != nullptr)
          wifiStatusNotifier(WiFiStatus::CONNECTING_ERROR);
        log(Logs::SIMPLE, "Cannot connect to WiFi, restarting ESP");
        __builtin_trap();
      }
      WiFi.begin(this->wifiSsid, this->wifiPassword);

      int iterations = 0;
      int connectionPulseTime = 0;
      if (wifiStatusNotifier != nullptr)
      {
        connectionPulseTime = this->wifiStatusConnectingPulseTime;
      }
      else
      {
        connectionPulseTime = 100;
      }

      iterations = WIFI_CONNECTING_MAX_TIME / connectionPulseTime;
      for (int i = 0; i < iterations; i++)
      {
        if (wifiStatusNotifier != nullptr)
          wifiStatusNotifier(WiFiStatus::CONNECTING);
        if (WiFi.status() == WL_CONNECTED)
        {
          if (wifiStatusNotifier != nullptr)
            wifiStatusNotifier(WiFiStatus::CONNECTED);
          log(Logs::SIMPLE, "Connected to WiFi!");
          return;
        }
        delay(connectionPulseTime);
      }

      counter++;
    }
  }
}

void Device::setWiFi(char const *ssid, char const *password)
{
  this->wifiSsid = ssid;
  this->wifiPassword = password;
  this->port = port;
  this->isWifiSetupEnabled = true;
}

void Device::setOTA(const char *password)
{
  this->OTAPassword = password;
  this->isOTAEnabled = true;
}

void Device::setupUDP()
{
  if (this->isUDPActive)
  {
    udp->begin(this->udpPort);
  }
}

void Device::start()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  log(Logs::SIMPLE, "");
  log(Logs::SIMPLE, "Starting thing...");
  log(Logs::SIMPLE, "MAC address:\t", false);
  log(Logs::SIMPLE, WiFi.macAddress().c_str());
  this->ensureHasWifi();
  WiFi.setAutoReconnect(true);
  log(Logs::SIMPLE, "IP address:\t", false);
  log(Logs::SIMPLE, WiFi.localIP().toString().c_str());
  if (this->isOTAEnabled)
  {
    ArduinoOTA.setHostname(this->name);
    ArduinoOTA.setPassword(this->OTAPassword);
    ArduinoOTA.begin();
  }

  this->webSocket = new CustomWebSocketsServer(this->port);
  this->webSocket->begin();

  auto eventFunction = [&](uint8_t num, WStype_t type, uint8_t *payload,
                           size_t length)
  {
    this->webSocketEvent(num, type, payload, length);
  };
  this->webSocket->onEvent(eventFunction);

  this->mainSender = new Sender(this->webSocket, clients);
  this->setupUDP();
}

Service *Device::findServiceWithId(String id)
{
  ServiceNode *serviceNode = this->serviceList;
  while (serviceNode->next != nullptr)
  {
    if (serviceNode->service->getId().equals(id))
    {
      return serviceNode->service;
    }
    serviceNode = serviceNode->next;
  }
  return nullptr;
}

void Device::addService(Service *service)
{
  service->setDevice(this);
  ServiceNode *newNode = new ServiceNode();
  newNode->service = service;
  newNode->next = this->serviceList;
  this->serviceList = newNode;
}

void Device::sendSimpleMessage(Sender *sender, HClient *client, String type)
{
  DynamicJsonDocument doc = this->prepareMessage(SMALL_MESSAGE_JSON_SIZE, type);
  String output;
  serializeJson(doc, output);
  sender->send(output, client);
}

bool Device::isFreeSpaceForNewClient()
{
  int spacesForClientTaken = 0;
  for (int i = 0; i < LIGHTHOUSE_CLIENT_MAX; i++)
  {
    if (!this->clients[i]->isEmpty())
    {
      spacesForClientTaken++;
    }
  }
  return spacesForClientTaken < (LIGHTHOUSE_CLIENT_MAX - 1);
}

void Device::logToDevices(const char *text)
{
  bool atLeastOneDeviceHasEnabledLogging = false;
  for (int i = 0; i < LIGHTHOUSE_CLIENT_MAX; i++)
  {
    if (this->clients[i]->isConnected() && this->clients[i]->isLogEnabled())
    {
      atLeastOneDeviceHasEnabledLogging = true;
      break;
    }
  }
  if (!atLeastOneDeviceHasEnabledLogging)
  {
    return;
  }
  DynamicJsonDocument doc(SMALL_MESSAGE_JSON_SIZE);
  doc["messageType"] = "log";
  doc["data"] = text;
  String output;
  serializeJson(doc, output);
  for (int i = 0; i < LIGHTHOUSE_CLIENT_MAX; i++)
  {
    if (this->clients[i]->isConnected() && this->clients[i]->isLogEnabled())
    {
      this->mainSender->send(output, this->clients[i]);
    }
  }
}

void Device::log(Logs logMode, const char *text, bool printNewline)
{
  if (!this->shouldLog(logMode))
    return;

  if (printNewline)
  {
    Serial.println(text);
  }
  else
  {
    Serial.print(text);
  }

  logToDevices(text);
}

size_t Device::logf(Logs logMode, const char *format, ...)
{
  if (!this->shouldLog(logMode))
    return 0;

  va_list arg;
  va_start(arg, format);
  char temp[64];
  char *buffer = temp;
  size_t len = vsnprintf(temp, sizeof(temp), format, arg);
  va_end(arg);
  if (len > sizeof(temp) - 1)
  {
    buffer = new char[len + 1];
    if (!buffer)
    {
      return 0;
    }
    va_start(arg, format);
    vsnprintf(buffer, len + 1, format, arg);
    va_end(arg);
  }
  len = Serial.write((const uint8_t *)buffer, len);
  logToDevices(buffer);
  if (buffer != temp)
  {
    delete[] buffer;
  }
  return len;
}

bool Device::shouldLog(Logs logMode)
{
  if (this->logsMode == Logs::DETAILED)
  {
    return true;
  }
  else if (this->logsMode == Logs::SIMPLE)
  {
    if (logMode == Logs::DETAILED)
      return false;
    if (logMode == Logs::SIMPLE)
      return true;
    return false;
  }
  else
  {
    return false;
  }
}

bool Device::isMessageProper(DynamicJsonDocument &json)
{
  return json.containsKey("messageType");
}

void Device::updateUDP()
{
  int packetSize = 0;
  do
  {
    packetSize = udp->parsePacket();
    if (packetSize)
    {
      logf(Logs::DETAILED, "Received %d bytes from %s, port %d\n", packetSize, udp->remoteIP().toString().c_str(), udp->remotePort());
      if (packetSize > 2000)
      {
        log(Logs::DETAILED, "Too long message!");
        return;
      }
      char *buffer = new char[packetSize + 1];
      int len = udp->read(buffer, packetSize);
      if (len > 0)
      {
        buffer[len] = '\0';
      }

      logf(Logs::DETAILED, "UDP packet contents: %s\n", buffer);

      HClient client(-1);
      UdpSender udpSender;
      Sender *tempSender = &udpSender;
      this->interpretMessage(&client, &udpSender, String(buffer));
      delete[] buffer;
    }
  } while (packetSize > 0);
}

void Device::sendUdpPacket(const char *ip, int port, const char *message)
{
  udp->beginPacket(ip, port);
  udp->print(message);
  delay(1);
  udp->endPacket();
  delay(1);
}

void Device::update()
{
  if (this->isOTAEnabled)
  {
    ArduinoOTA.handle();
  }

  if (this->isWifiSetupEnabled)
  {
    this->ensureHasWifi();
  }

  if (this->isUDPActive)
  {
    this->updateUDP();
  }

  this->webSocket->loop();

  ServiceNode *serviceNode = this->serviceList;
  while (serviceNode->next != nullptr)
  {
    serviceNode->service->update(this->mainSender);
    serviceNode = serviceNode->next;
  }

  for (int i = 0; i < LIGHTHOUSE_CLIENT_MAX; i++)
  {
    if (this->clients[i]->isConnected())
    {
      if (this->clients[i]->isKeepaliveTimeout())
      {
        sendSimpleMessage(this->mainSender, this->clients[i], "keepaliveTimeout");
        this->webSocket->disconnect(this->clients[i]->getSocketId());
      }
    }
  }
}
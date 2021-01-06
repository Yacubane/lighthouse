#include "LightDevice.h"

Device::Device(String name)
{
    this->name = name;

    this->propertyList = new PropertyNode();
    this->propertyList->property = nullptr;
    this->propertyList->next = nullptr;

    this->actionList = new ActionNode();
    this->actionList->action = nullptr;
    this->actionList->next = nullptr;
}

DynamicJsonDocument Device::prepareMessage(int capacity, String type)
{
    DynamicJsonDocument doc(capacity);
    doc["messageType"] = type;
    return doc;
}

void Device::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected\n", num);
        clients[num].setDisconnected();
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = this->webSocket->remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        if (!this->isFreeSpaceForNewClient())
        {
            clients[num].setConnected();
            this->sendText(clients[num], "{ \"messageType\": \"noSpace\" }");
            this->webSocket->disconnect(num);
            clients[num].setDisconnected();
        }
        else
        {
            clients[num].setConnected();
        }
    }
    break;
    case WStype_TEXT:
    {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        String message = (char *)payload;
        this->interpretMessage(this->clients[num], message);
    }
    break;
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

void Device::interpretMessage(HClient &client, String message)
{
    Serial.printf("Interpreting message from client with ID: %d\n", client.getId());

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

                    Serial.printf("Parsing JSON: %s\n", jsonMessage.c_str());

                    DynamicJsonDocument doc(JSON_CAPACITY);
                    DeserializationError deserializationError = deserializeJson(doc, jsonMessage);
                    if (deserializationError.code() == deserializationError.Ok)
                    {
                        this->interpretMessage(client, doc);
                        foundMessage = true;
                    }
                    else
                    {
                        Serial.printf("Error parsing JSON: %s\n", deserializationError.c_str());
                    }

                    break;
                }
            }
        }
        // could not find json message :(
        if (!foundMessage)
        {
            Serial.println("Could not find JSON message!");
            return;
        }
    }
}

void Device::interpretMessage(HClient &client, DynamicJsonDocument &json)
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
            client.setAuthenticated(true);
            sendSimpleMessage(client, "authenticationSuccess");
        }
        else
        {
            sendSimpleMessage(client, "authenticationFail");
        }
    }
    else if (messageType.equals("keepalive"))
    {
        client.keepalive();
    }
    else if (messageType.equals("ping"))
    {
        sendSimpleMessage(client, "pong");
    }
    else if (messageType.equals("describe"))
    {
        //TODO
    }
    else if (messageType.equals("requestAction"))
    {
        if (!client.isAuthenticated())
        {
            return;
        }
        if (!json.containsKey("data"))
        {
            return;
        }
        JsonObject data = json["data"];
        if (!data.containsKey("name"))
        {
            return;
        }
        String actionName = data["name"];
        Action *action = findActionWithName(actionName);
        if (action == nullptr)
        {
            return;
        }
        if (!action->areArgumentsValid(data))
        {
            return;
        }

        action->invokeAction(data);
    }
    else if (messageType.equals("subscribeEverything"))
    {
        if (!client.isAuthenticated())
        {
            return;
        }
        client.setHasSubscribedToEverything(true);
    }
    else if (messageType.equals("readAllProperties"))
    {
        if (!client.isAuthenticated())
        {
            return;
        }
        PropertyNode *propertyNode = this->propertyList;
        while (propertyNode->next != nullptr)
        {
            {
                DynamicJsonDocument doc = this->prepareMessage(200, "propertyStatus");
                JsonObject data = doc.createNestedObject("data");
                propertyNode->property->addToJson(data);
                String output;
                serializeJson(doc, output);
                this->sendText(client, output);
            }
            propertyNode = propertyNode->next;
        }
    }
}

void Device::setup(String ssid, String password, int port)
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
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

void Device::start()
{
    this->webSocket = new WebSocketsServer(this->port);
    this->webSocket->begin();

    auto eventFunction = [&](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        this->webSocketEvent(num, type, payload, length);
    };
    this->webSocket->onEvent(eventFunction);

    this->webSocket->enableHeartbeat(10000, 5000, 1);
}

void Device::addProperty(Property &property)
{
    PropertyNode *newNode = new PropertyNode();
    newNode->property = &property;
    newNode->next = this->propertyList;
    this->propertyList = newNode;
}

void Device::addAction(String name, void (*handler)())
{
    ActionNode *newNode = new ActionNode();
    Action *action = new Action(name);
    action->setSimpleHandler(handler);
    newNode->action = action;
    newNode->next = this->actionList;
    this->actionList = newNode;
}

void Device::broadcastText(String text)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (this->clients[i].isConnected() && this->clients[i].hasSubscribedToEverything())
        {
            this->sendText(this->clients[i], text);
        }
    }
}

void Device::sendText(HClient &client, String text)
{
    this->webSocket->sendTXT(client.getId(), text);
}

void Device::broadcastEvent(String name)
{
    DynamicJsonDocument doc = this->prepareMessage(200, "event");
    JsonObject data = doc.createNestedObject("data");
    data["name"] = name;
    String output;
    serializeJson(doc, output);
    broadcastText(output);
}

void Device::sendSimpleMessage(HClient &client, String type)
{
    DynamicJsonDocument doc = this->prepareMessage(200, type);
    String output;
    serializeJson(doc, output);
    this->sendText(client, output);
}

bool Device::isFreeSpaceForNewClient()
{
    int clientsConnected = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (this->clients[i].isConnected())
        {
            clientsConnected++;
        }
    }
    Serial.print("Connected: ");
    Serial.println(clientsConnected);
    return clientsConnected < (MAX_CLIENTS - 1);
}

bool Device::isMessageProper(DynamicJsonDocument &json)
{
    return json.containsKey("messageType");
}

Action *Device::findActionWithName(String name)
{
    ActionNode *actionNode = this->actionList;
    while (actionNode->next != nullptr)
    {
        if (actionNode->action->getName().equals(name))
        {
            return actionNode->action;
        }
        actionNode = actionNode->next;
    }
    return nullptr;
}

void Device::update()
{
    this->webSocket->loop();
    PropertyNode *propertyNode = this->propertyList;
    while (propertyNode->next != nullptr)
    {
        if (propertyNode->property->isChanged())
        {
            propertyNode->property->setChanged(false);
            DynamicJsonDocument doc = this->prepareMessage(200, "propertyChanged");
            JsonObject data = doc.createNestedObject("data");
            propertyNode->property->addToJson(data);
            String output;
            serializeJson(doc, output);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (this->clients[i].isConnected() && this->clients[i].hasSubscribedToEverything())
                {
                    this->sendText(this->clients[i], output);
                }
            }
        }
        propertyNode = propertyNode->next;
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (this->clients[i].isConnected())
        {
            if (this->clients[i].isKeepaliveTimeout())
            {
                sendSimpleMessage(this->clients[i], "keepaliveTimeout");
                this->webSocket->disconnect(this->clients[i].getId());
            }
        }
    }
}
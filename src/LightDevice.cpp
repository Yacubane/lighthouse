#include "LightDevice.h"

Device::Device(String name) {
    this->name = name; 

    this->propertyList = new PropertyNode();
    this->propertyList->property = nullptr;
    this->propertyList->next = nullptr;

    this->actionList = new ActionNode();
    this->actionList->action = nullptr;
    this->actionList->next = nullptr;
}

DynamicJsonDocument Device::prepareMessage(int capacity, String type) {
    DynamicJsonDocument doc(capacity);
    doc["messageType"] = type;
    return doc;
}

void Device::setup(String ssid, String password, int port) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("");
    WiFi.setAutoReconnect(true);
    Serial.print("IP address:\t"); Serial.print(WiFi.localIP()); Serial.print("\tPort:\t"); Serial.println(port);
    this->port = port;
}

void Device::start() {
    this->server = new WiFiServer(this->port);
    this->server->begin();
}

void Device::addProperty(Property& property) {
        PropertyNode* newNode = new PropertyNode();
        newNode->property = &property;
        newNode->next = this->propertyList;
        this->propertyList = newNode;
    }

void Device::addAction(String name, void (*handler)()){
    ActionNode* newNode = new ActionNode();
    Action* action = new Action(name);
    action->setSimpleHandler(handler);
    newNode->action = action;
    newNode->next = this->actionList;
    this->actionList = newNode;
}

void Device::broadcastMessage(String text) {
    for (int i = 0; i < 4; i++) {
        if (this->clients[i].isConnected() && this->clients[i].hasSubscribedToEverything()) {
            this->clients[i].sendString(text);
        }
    }
}

void Device::broadcastEvent(String name) {
    DynamicJsonDocument doc = this->prepareMessage(200, "event");
    JsonObject data = doc.createNestedObject("data");
    data["name"] = name;
    String output;
    serializeJson(doc, output);
    broadcastMessage(output);
}

void Device::sendSimpleMessage(HClient client, String type) {
    DynamicJsonDocument doc = this->prepareMessage(200, type);
    String output;
    serializeJson(doc, output);
    client.sendString(output);
}

int Device::getFirstFreeClientIndex() {
    for (int i = 0; i < 4; i++) {
        if (!this->clients[i].isConnected()) {
            return i;
        }
    }
    return -1;
}

bool Device::isMessageProper(DynamicJsonDocument json) {
    return json.containsKey("messageType");
}

Action* Device::findActionWithName(String name) {
    ActionNode* actionNode = this->actionList;
    while (actionNode->next != nullptr) {
        if (actionNode->action->getName().equals(name)) {
            return actionNode->action;
        }
        actionNode = actionNode->next;
    }
    return nullptr;
}    

void Device::update() {
    PropertyNode* propertyNode = this->propertyList;
    while (propertyNode->next != nullptr) {
        if (propertyNode->property->isChanged()) {
            propertyNode->property->setChanged(false);
            DynamicJsonDocument doc = this->prepareMessage(200, "propertyChanged");
            JsonObject data = doc.createNestedObject("data");
            propertyNode->property->addToJson(data);
            String output;
            serializeJson(doc, output);
            for (int i = 0; i < 4; i++) {
                if (this->clients[i].isConnected() && this->clients[i].hasSubscribedToEverything()) {
                    this->clients[i].sendString(output);
                }
            }
        }
        propertyNode = propertyNode->next;
    }

    WiFiClient newClient = this->server->available();
    if (newClient) {
        int index = getFirstFreeClientIndex();
        if (index == -1) {
            newClient.println("{ \"messageType\": \"noSpace\"");
            newClient.stop();
        } else {
            clients[index].setClient(newClient);
            if (devicePassword.equals("")) {
                clients[index].setAuthenticated(true);
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        while (this->clients[i].isConnected() && this->clients[i].hasPendingMessage()) {
            DynamicJsonDocument json = clients[i].receiveJsonObject();
            if (!isMessageProper(json)) {
                continue;
            }
            String messageType = json["messageType"];

            if (messageType.equals("handshake")) {
                if (!json.containsKey("data")) {
                    continue;
                }
                JsonObject data = json["data"];
                if (!data.containsKey("password")) {
                    continue;
                }
                String password = data["password"];
                if (password.equals(this->devicePassword)) {
                    this->clients[i].setAuthenticated(true);
                    sendSimpleMessage(this->clients[i], "authenticationSuccess");
                } else {
                    sendSimpleMessage(this->clients[i], "authenticationFail");
                }

            } else if (messageType.equals("keepalive")) {
                clients[i].keepalive();
            } else if (messageType.equals("describe")) {
                //TODO
            } else if (messageType.equals("requestAction")) {
                if (this->clients[i].isAuthenticated()) {
                    if (!json.containsKey("data")) {
                        continue;
                    }
                    JsonObject data = json["data"];
                    if (!data.containsKey("name")) {
                        continue;
                    }
                    String actionName = data["name"];
                    Action* action = findActionWithName(actionName);
                    if (action == nullptr) {
                        continue;
                    }
                    if (!action->areArgumentsValid(data)) {
                        continue;
                    }

                    action->invokeAction(data);
                }
            } else if (messageType.equals("subscribeEverything")) {
                if (this->clients[i].isAuthenticated()) {
                    this->clients[i].setHasSubscribedToEverything(true);    
                }
            } else if (messageType.equals("readAllProperties")) {
                if (this->clients[i].isAuthenticated()) {
                    PropertyNode* propertyNode = this->propertyList;
                    while (propertyNode->next != nullptr) {
                        {
                            DynamicJsonDocument doc = this->prepareMessage(200, "propertyStatus");
                            JsonObject data = doc.createNestedObject("data");
                            propertyNode->property->addToJson(data);
                            String output;
                            serializeJson(doc, output);
                            this->clients[i].sendString(output);
                        }
                        propertyNode = propertyNode->next;
                    }
                }
            }              
        }
    }

    for (int i = 0; i < 4; i++) {
        if (this->clients[i].isConnected()) {
            if (this->clients[i].isKeepaliveTimeout()) {
                sendSimpleMessage(this->clients[i], "keepaliveTimeout");
                this->clients[i].disconnect();
            } else if (this->clients[i].hasError()) {
                sendSimpleMessage(this->clients[i], "error");
                this->clients[i].disconnect();
            }
        } 
    }
}
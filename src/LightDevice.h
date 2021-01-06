#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "LightProperty.h"
#include "LightAction.h"
#include "LightClient.h"
#include <WebSocketsServer.h>

#define MAX_CLIENTS 5

struct PropertyNode {
    Property* property;
    PropertyNode* next;
};

struct ActionNode {
    Action* action;
    ActionNode* next;
};

class Device {
public:
    Device(String name);

    void setup(String ssid, String password, int port);
    void update();
    void start();

    void addProperty(Property& property);
    void addAction(String name, void (*handler)());

    void broadcastEvent(String name);

    void setPassword(String password) {
        this->devicePassword = password;
    }

    void interpretMessage(HClient& client, String message);
    void interpretMessage(HClient& client, DynamicJsonDocument& json);

private:
    String name;
    int port;
    PropertyNode* propertyList;
    ActionNode* actionList;
    WebSocketsServer* webSocket;
    HClient clients[5] = {{0}, {1}, {2}, {3}, {4}};
    String fragmentBuffer[5];
    String devicePassword = "";


    void sendSimpleMessage(HClient& client, String type);
    bool isFreeSpaceForNewClient();
    bool isMessageProper(DynamicJsonDocument& json);
    Action* findActionWithName(String name);
    void broadcastText(String text);
    void sendText(HClient& client, String text);
    DynamicJsonDocument prepareMessage(int capacity, String type);
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
};
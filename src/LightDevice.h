#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "LightProperty.h"
#include "LightAction.h"
#include "LightClient.h"

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

private:
    String name;
    int port;
    PropertyNode* propertyList;
    ActionNode* actionList;
    WiFiServer* server;
    HClient clients[4];
    String devicePassword = "";


    void sendSimpleMessage(HClient client, String type);
    int getFirstFreeClientIndex();
    bool isMessageProper(DynamicJsonDocument json);
    Action* findActionWithName(String name);
    void broadcastMessage(String text);
    DynamicJsonDocument prepareMessage(int capacity, String type);
};
#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "LightProperty.h"
#include "LightAction.h"
#include "LightClient.h"
#include "LightDevice.h"
#include "LightSender.h"
#include <WebSocketsServer.h>

#define MAX_CLIENTS 5

class Device;

struct PropertyNode
{
    Property *property;
    PropertyNode *next;
};

struct ActionNode
{
    Action *action;
    ActionNode *next;
};

class Service
{
public:
    Service(String id, std::vector<const char*> types, String description);

    void update(Sender *sender);

    void addProperty(Property &property);
    void addAction(String id, std::vector<const char*> types, String description, void (*handler)(ActionStatus *actionStatus, JsonObject jsonObject));

    void interpretMessage(HClient &client, Sender *sender, JsonObject &json);

    void setDevice(Device *device)
    {
        this->device = device;
    }

    String getId()
    {
        return id;
    }

private:
    String id;
    std::vector<const char*> types;
    int typesLength;
    String description;

    PropertyNode *propertyList;
    ActionNode *actionList;

    Device *device;

    Action *findActionWithId(String id);
    DynamicJsonDocument prepareMessage(int capacity, String type);
};
#pragma once

#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "LightProperty.h"
#include "LightAction.h"
#include "LightClient.h"
#include "LightService.h"
#include "LightSender.h"
#include "LightUDPSender.h"
#include <WebSocketsServer.h>
#include "LightDefines.h"

#define MAX_CLIENTS 5

class Service;

struct ServiceNode
{
    Service *service;
    ServiceNode *next;
};

class Device
{
public:
    Device(char *name, int port);

    void setWiFi(String ssid, String password);
    void setOTA(const char *password);
    void setUDPSupport(int port);
    void update();
    void start();

    void addService(Service *service);

    void setPassword(String password)
    {
        this->devicePassword = password;
    }

    void interpretMessage(HClient &client, Sender* sender, String message);
    void interpretMessage(HClient &client, Sender* sender, DynamicJsonDocument &json);
    void sendUdpPacket(const char* ip, int port, const char* message);
private:
    char *name;
    int port;
    unsigned long clientsCounter;
    ServiceNode *serviceList;
    WebSocketsServer *webSocket;
    HClient clients[5] = {{0}, {1}, {2}, {3}, {4}};
    String fragmentBuffer[5];
    String devicePassword = "";
    Sender *mainSender;
    WiFiUDP *udp;
    int udpPort;

    String wifiSsid;
    String wifiPassword;
    bool isOTAEnabled;
    bool isWifiSetupEnabled;
    bool isUDPActive;

    void sendSimpleMessage(Sender* sender, HClient &client, String type);
    bool isFreeSpaceForNewClient();
    bool isMessageProper(DynamicJsonDocument &json);
    Action *findActionWithName(String name);
    void broadcastText(String text);
    DynamicJsonDocument prepareMessage(int capacity, String type);
    void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
    Service *findServiceWithId(String id);
    void ensureHasWifi();
    void setupUDP();
    void updateUDP();
};
#pragma once

#include <Arduino.h>
#include "LighthouseImports.h"
#include <ArduinoOTA.h>
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
#include "DiagnosticService.h"

#define MAX_CLIENTS 5
#define WIFI_CONNECTING_MAX_TIME 20000 // 20000ms = 20s

class Service;

struct ServiceNode
{
    Service *service;
    ServiceNode *next;
};

class Device
{
public:
    enum WiFiStatus
    {
        CONNECTED,
        DISCONNECTED,
        CONNECTING,
        CONNECTING_ERROR
    };
    enum Logs
    {
        NONE,
        SIMPLE,
        DETAILED
    };
    Device(char const *name, int port);

    void setWiFi(char const *ssid, char const *password);
    void setOTA(const char *password);
    void setUDPSupport(int port);
    void setWifiStatusNotifier(void (*wifiStatusHandler)(WiFiStatus status), int connectingPulseTime);
    void update();
    void start();

    void addService(Service *service);

    void setPassword(String password)
    {
        this->devicePassword = password;
    }

    void setLogsMode(Logs logsMode)
    {
        this->logsMode = logsMode;
    }

private:
    char const *name;
    unsigned int port;
    Logs logsMode;
    unsigned long clientsCounter;
    ServiceNode *serviceList;
    WebSocketsServer *webSocket;
    HClient clients[5] = {{0}, {1}, {2}, {3}, {4}};
    String fragmentBuffer[5];
    String devicePassword = "";
    Sender *mainSender;
    WiFiUDP *udp;
    int udpPort;

    DiagnosticService *diagnosticService;
    char const *wifiSsid;
    char const *wifiPassword;
    const char *OTAPassword;
    bool isOTAEnabled;
    bool isWifiSetupEnabled;
    bool isUDPActive;
    int wifiStatusConnectingPulseTime;
    void (*wifiStatusNotifier)(WiFiStatus status);

    void sendSimpleMessage(Sender *sender, HClient &client, String type);
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
    void interpretMessage(HClient &client, Sender *sender, String message);
    void interpretMessage(HClient &client, Sender *sender, DynamicJsonDocument &json);
    void sendUdpPacket(const char *ip, int port, const char *message);
    void log(Logs logMode, const char *text, bool printNewLine = true);
    size_t logf(Logs logMode, const char *format, ...);
    void logToDevices(const char *text);
    bool shouldLog(Logs logMode);
};
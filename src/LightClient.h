#pragma once
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#define READ_TIMEOUT 50
#define JSON_CAPACITY 2048

class HClient
{
public:
    HClient(int id)
    {
        this->id = id;
        this->connected = false;
    }

    void setConnected()
    {
        this->keepalive();
        this->authenticated = false;
        this->connected = true;
        this->subscribedToEverything = false;
    }

    void setDisconnected()
    {
        this->connected = false;
    }

    bool isConnected()
    {
        return this->connected;
    }

    bool hasSubscribedToEverything()
    {
        return subscribedToEverything;
    }

    void setHasSubscribedToEverything(bool subscribedToEverything)
    {
        this->subscribedToEverything = subscribedToEverything;
    }

    int getId()
    {
        return id;
    }

    bool isAuthenticated()
    {
        return this->authenticated;
    }

    void setAuthenticated(bool authenticated)
    {
        this->authenticated = authenticated;
    }

    void keepalive()
    {
        this->lastKeepalive = millis();
    }

    bool isKeepaliveTimeout()
    {
        return millis() - lastKeepalive > 15000;
    }

private:
    bool connected;
    bool subscribedToEverything;
    int id;
    bool authenticated;
    unsigned long lastKeepalive;
};
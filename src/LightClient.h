#pragma once
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "LightDefines.h"

#define READ_TIMEOUT 50

class HClient
{
public:
    HClient(int socketId)
    {
        this->socketId = socketId;
        this->connected = false;
    }

    void setConnected()
    {
        this->keepalive();
        this->authenticated = false;
        this->connected = true;
    }

    void setDisconnected()
    {
        this->connected = false;
    }

    bool isConnected()
    {
        return this->connected;
    }

    void setId(String id) {
        this->id = id;
    }

    String getId()
    {
        return id;
    }

    int getSocketId()
    {
        return socketId;
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
    String id;
    unsigned int socketId;
    bool authenticated;
    unsigned long lastKeepalive;
};
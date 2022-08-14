#pragma once
#include <Arduino.h>
#include "LighthouseImports.h"
#include <ArduinoJson.h>
#include "LightDefines.h"

class HClient
{
public:
    HClient(int socketId)
    {
        this->socketId = socketId;
        this->logEnabled = false;
        this->authenticated = false;
        this->connected = false;
        this->empty = true;
    }

    void setLogEnabled(bool logEnabled) {
        this->logEnabled = logEnabled;
    }

    bool isLogEnabled() {
        return this->logEnabled;
    }

    void setConnected()
    {
        this->keepalive();
        this->authenticated = false;
        this->logEnabled = false;
        this->connected = true;
    }

    void setEmpty(bool empty) {
        this->empty = empty;
    }

    bool isEmpty() {
        return this->empty;
    }

    void setDisconnected()
    {
        this->connected = false;
    }

    bool isConnected()
    {
        return this->connected;
    }

    void setId(String id)
    {
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
    bool logEnabled;
    bool empty;
};
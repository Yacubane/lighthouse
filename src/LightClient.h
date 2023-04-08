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
        this->subscribedToAllProperties = LIGHTHOUSE_NEW_CLIENT_AUTOMATICALLY_SUBSCRIBED_TO_ALL_PROPERTIES;
        this->subscribedToAllDebugProperties = false;
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
        return millis() - lastKeepalive > LIGHTHOUSE_CLIENT_KEEPALIVE_TIMEOUT;
    }

    void setSubscribedToAllProperties(bool subscribedToAllProperties) {
        this->subscribedToAllProperties = subscribedToAllProperties;
    }

    void setSubscribedToAllDebugProperties(bool subscribedToAllDebugProperties) {
        this->subscribedToAllDebugProperties = subscribedToAllDebugProperties;
    }

    bool isSubscribedToAllProperties() {
        return subscribedToAllProperties;
    }

    bool isSubscribedToAllDebugProperties() {
        return subscribedToAllDebugProperties;
    }

private:
    bool connected;
    String id;
    unsigned int socketId;
    bool authenticated;
    unsigned long lastKeepalive;
    bool logEnabled;
    bool empty;
    bool subscribedToAllProperties;
    bool subscribedToAllDebugProperties;
};
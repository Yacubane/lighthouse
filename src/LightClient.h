#pragma once
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#define READ_TIMEOUT 50
#define JSON_CAPACITY 2048

class HClient {
public:
    HClient() { 
        this->active = false; 
    }

    void setClient(WiFiClient client);

    void disconnect();

    bool isConnected() { 
        return this->active && this->wifiClient.connected(); 
    }

    DynamicJsonDocument receiveJsonObject();

    void sendString(String message) { 
        this->wifiClient.print(message); 
    }

    bool hasPendingMessage() { 
        return this->wifiClient.available(); 
    }

    bool hasSubscribedToEverything() { 
        return subscribedToEverything; 
    }

    bool isAuthenticated() { 
        return this->authenticated; 
    }

    void setAuthenticated(bool authenticated) { 
        this->authenticated = authenticated; 
    }

    void setHasSubscribedToEverything(bool subscribedToEverything) { 
        this->subscribedToEverything = subscribedToEverything; 
    }

    void keepalive() { 
        this->lastKeepalive = millis(); 
    }

    bool isKeepaliveTimeout() { 
        return millis() - lastKeepalive > 15000; 
    }

    bool hasError() { 
        return error; 
    }

private:
    WiFiClient wifiClient;
    bool active;
    bool error;
    bool authenticated;
    bool subscribedToEverything;
    unsigned long lastKeepalive;

    String setError();
    int timedRead();
    String receiveJsonObjectString();
};
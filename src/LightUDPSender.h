#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>
#include "LightClient.h"
#include "LightDefines.h"
#include "LightSender.h"

class UdpSender : public Sender
{
public:
    UdpSender() : Sender(){}

    void send(String text, HClient &client) override {
        Serial.print("UDP to client: ");
        Serial.println(text);
    }

    void sendAll(String text) override {
        Serial.print("UDP to all: ");
        Serial.println(text);
    }

    HClient *getClients()
    {
        return nullptr;
    }
};
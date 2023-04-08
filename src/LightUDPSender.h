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

    void send(String text, HClient *client) override {
        // fake interface used when interpreting UDP messages - no message will be send to other devices when using this type of communication
    }

    void sendAll(String text) override {
        // fake interface used when interpreting UDP messages - no message will be send to other devices when using this type of communication
    }

    void sendToAllPropertySubscribers(String text, bool debugSubscribersOnly) override {
        // fake interface used when interpreting UDP messages - no message will be send to other devices when using this type of communication
    }

    HClient **getClients()
    {
        return nullptr;
    }
};
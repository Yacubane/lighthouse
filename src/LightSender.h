#pragma once

#include <Arduino.h>
#include "CustomWebSocketsServer.h"
#include "LightClient.h"
#include "LightDefines.h"

class Sender
{
public:
    Sender(){}

    Sender(CustomWebSocketsServer *webSocket, HClient **clients)
    {
        this->webSocket = webSocket;
        this->clients = clients;
    }

    virtual void send(String text, HClient *client)
    {
        if(!this->webSocket->sendTXT(client->getSocketId(), text)) {
            this->webSocket->disconnect(client->getSocketId());
            client->setDisconnected();
        }
    }

    virtual void send(HClient *client, WSopcode_t opcode, uint8_t * payload, size_t length, bool fin, bool headerToPayload)
    {
        if(!this->webSocket->sendFrame(client->getSocketId(), opcode, payload, length, fin, headerToPayload)) {
            this->webSocket->disconnect(client->getSocketId());
            client->setDisconnected();
        }
    }

    virtual void sendAll(String text)
    {
        for (int i = 0; i < LIGHTHOUSE_CLIENT_MAX; i++)
        {
            if (this->clients[i]->isConnected() && this->clients[i]->isAuthenticated())
            {
                this->send(text, this->clients[i]);
            }
        }
    }

    virtual void sendToAllPropertySubscribers(String text, bool debugSubscribersOnly)
    {
        for (int i = 0; i < LIGHTHOUSE_CLIENT_MAX; i++)
        {
            if (this->clients[i]->isConnected() && this->clients[i]->isAuthenticated() &&
                 this->clients[i]->isSubscribedToAllProperties() && 
                 (!debugSubscribersOnly || this->clients[i]->isSubscribedToAllDebugProperties()))
            {
                this->send(text, this->clients[i]);
            }
        }
    }

    virtual HClient **getClients()
    {
        return clients;
    }

private:
    CustomWebSocketsServer *webSocket;
    HClient **clients;
};
#pragma once

#include <Arduino.h>
#include "CustomWebSocketsServer.h"
#include "LightClient.h"
#include "LightDefines.h"

#define MAX_CLIENTS 5

class Sender
{
public:
    Sender(){}

    Sender(CustomWebSocketsServer *webSocket, HClient *clients)
    {
        this->webSocket = webSocket;
        this->clients = clients;
    }

    virtual void send(String text, HClient &client)
    {
        this->webSocket->sendTXT(client.getSocketId(), text);
    }

    virtual void send(HClient *client, WSopcode_t opcode, uint8_t * payload, size_t length, bool fin, bool headerToPayload)
    {
        this->webSocket->sendFrame(client->getSocketId(), opcode, payload, length, fin, headerToPayload);   
    }

    virtual void sendAll(String text)
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (this->clients[i].isConnected() && this->clients[i].isAuthenticated())
            {
                this->send(text, this->clients[i]);
            }
        }
    }

    virtual HClient *getClients()
    {
        return clients;
    }

private:
    CustomWebSocketsServer *webSocket;
    HClient *clients;
};
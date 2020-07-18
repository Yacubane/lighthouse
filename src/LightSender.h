#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>
#include "LightClient.h"
#include "LightDefines.h"

#define MAX_CLIENTS 5

class Sender
{
public:
    Sender(){}

    Sender(WebSocketsServer *webSocket, HClient *clients)
    {
        this->webSocket = webSocket;
        this->clients = clients;
    }

    virtual void send(String text, HClient &client)
    {
        this->webSocket->sendTXT(client.getSocketId(), text);
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
    WebSocketsServer *webSocket;
    HClient *clients;
};
#pragma once
#include <WebSocketsServer.h>

// WebSocketsServer with unlocked protected API to sendFrame
class CustomWebSocketsServer : public WebSocketsServer
{
public:
    using WebSocketsServer::WebSocketsServer;
    bool sendFrame(uint8_t num, WSopcode_t opcode, uint8_t *payload, size_t length, bool fin, bool headerToPayload)
    {
        if (num >= WEBSOCKETS_SERVER_CLIENT_MAX)
        {
            return false;
        }
        WSclient_t *client = &_clients[num];
        if (clientIsConnected(client))
        {
            return this->sendFrame(client, opcode, payload, length, fin, headerToPayload);
        }
        return false;
    }

private:
    using WebSocketsServer::sendFrame;
};
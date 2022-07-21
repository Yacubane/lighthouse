#pragma once

#include <Arduino.h>
#include "LightProperty.h"
#include "LightDefines.h"
#include "LightClient.h"

class ActionStatus
{

public:
    enum Status
    {
        PENDING,
        ERROR,
        COMPLETED
    };

    ActionStatus(String id, String actionId, String requestId, HClient* client, Status status, String message, String userMessage)
    {
        this->id = id;
        this->actionId = actionId;
        this->requestId = requestId;
        this->client = client;
        // After client disconnection client reference can be changed to another
        // with different clientId (incremented for every new client) so it must be stored
        // to be able to differentiate these clients
        this->clientId = client->getId();
        this->status = status;
        this->message = message;
        this->userMessage = userMessage;
        this->changed = false;
    }

    String getId()
    {
        return this->id;
    }

    String getActionId()
    {
        return this->actionId;
    }

    String getRequestId()
    {
        return this->requestId;
    }

    String getClientId()
    {
        return this->clientId;
    }

    HClient* getClient()
    {
        return this->client;
    }

    Status getStatus()
    {
        return this->status;
    }

    String getMessage()
    {
        return this->message;
    }

    String getUserMessage()
    {
        return this->userMessage;
    }

    void set(Status status, String message, String userMessage)
    {
        this->status = status;
        this->message = message;
        this->userMessage = userMessage;
        this->changed = true;
    }

    bool isChanged()
    {
        return this->changed;
    }

    void setChanged(bool flag)
    {
        this->changed = flag;
    }

private:
    String id;
    String actionId;
    HClient* client;
    String clientId;
    String requestId;
    Status status;
    String message;
    String userMessage;
    bool changed;
};
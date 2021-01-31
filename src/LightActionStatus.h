#pragma once

#include <Arduino.h>
#include "LightProperty.h"

class ActionStatus
{

public:
    enum Status
    {
        PENDING,
        ERROR,
        COMPLETED
    };

    ActionStatus(String id, String actionId, String requestId, String clientId, Status status, String message, String userMessage)
    {
        this->id = id;
        this->actionId = actionId;
        this->requestId = requestId;
        this->clientId = clientId;
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
    String clientId;
    String requestId;
    Status status;
    String message;
    String userMessage;
    bool changed;
};
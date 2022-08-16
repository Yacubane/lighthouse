#pragma once

#include <Arduino.h>
#include "LightProperty.h"
#include "LightActionStatus.h"
#include "LightClient.h"
#include "LightDefines.h"
#include "LightSender.h"

struct ActionStatusNode
{
    ActionStatus *actionStatus;
    ActionStatusNode *next;
};

class Action
{
public:
    Action(const char *id, std::vector<const char *> semanticTypes, const char *description)
    {
        this->id = id;
        this->semanticTypes = semanticTypes;
        this->description = description;
        this->counter = 0;

        this->actionStatusList = new ActionStatusNode();
        this->actionStatusList->actionStatus = nullptr;
        this->actionStatusList->next = nullptr;
    }

    const char *getId()
    {
        return this->id;
    }

    const char *getDescription()
    {
        return this->description;
    }

    void setHandler(void (*handler)(ActionStatus *actionStatus, JsonVariant data))
    {
        this->handler = handler;
    }

    void (*getHandler(void))(ActionStatus *actionStatus, JsonVariant data)
    {
        return this->handler;
    }

    bool areArgumentsValid(JsonVariant data)
    {
        return true;
    }

    ActionStatus *invokeAction(HClient *client, Sender* sender, const char *requestId, JsonVariant data)
    {
        ActionStatus *actionStatus = new ActionStatus(String(counter++), getId(), requestId, client, sender, ActionStatus::PENDING, "PENDING", "Starting action");
        actionStatus->setChanged(true);
        ActionStatusNode *newNode = new ActionStatusNode();
        newNode->actionStatus = actionStatus;
        newNode->next = this->actionStatusList->next;
        this->actionStatusList->next = newNode;

        return actionStatus;
    }

    ActionStatusNode *getActionStatusList()
    {
        return actionStatusList;
    }

    std::vector<const char *> getSemanticTypes()
    {
        return this->semanticTypes;
    }

private:
    const char *id;
    const char *description;
    ActionStatusNode *actionStatusList;
    std::vector<const char *> semanticTypes;
    unsigned long counter;
    void (*handler)(ActionStatus *actionStatus, JsonVariant data);
};
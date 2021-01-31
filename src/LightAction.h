#pragma once

#include <Arduino.h>
#include "LightProperty.h"
#include "LightActionStatus.h"
#include "LightClient.h"

struct ActionStatusNode
{
    ActionStatus *actionStatus;
    ActionStatusNode *next;
};

class Action
{
public:
    Action(String id, std::vector<const char*> types, String description)
    {
        this->id = id;
        this->types = types;
        this->description = description;
        this->counter = 0;

        this->actionStatusList = new ActionStatusNode();
        this->actionStatusList->actionStatus = nullptr;
        this->actionStatusList->next = nullptr;
    }

    String getId()
    {
        return this->id;
    }

    void setHandler(void (*handler)(ActionStatus *actionStatus, JsonObject data))
    {
        this->handler = handler;
    }

    void (*getHandler(void))(ActionStatus *actionStatus, JsonObject data) {
        return this->handler;
    }

    bool areArgumentsValid(JsonObject data)
    {
        return true;
    }

    ActionStatus* invokeAction(HClient& client, JsonObject data)
    {
        ActionStatus *actionStatus = new ActionStatus(String(counter++), getId(), data["requestId"], client.getId(), ActionStatus::PENDING, "PENDING", "Starting action");
        actionStatus->setChanged(true);
        ActionStatusNode *newNode = new ActionStatusNode();
        newNode->actionStatus = actionStatus;
        newNode->next = this->actionStatusList->next;
        this->actionStatusList->next = newNode;

        return actionStatus;
    }

    ActionStatusNode* getActionStatusList() {
        return actionStatusList;
    }


private:
    String id;
    String description;
    ActionStatusNode *actionStatusList;
    std::vector<const char*> types;
    unsigned long counter;
    void (*handler)(ActionStatus *actionStatus, JsonObject data);
};
#include "LightService.h"

Service::Service(String id, std::vector<const char *> types, String description)
{
    this->id = id;
    this->types = types;
    this->typesLength = typesLength;
    this->description = description;

    this->propertyList = new PropertyNode();
    this->propertyList->property = nullptr;
    this->propertyList->next = nullptr;

    this->actionList = new ActionNode();
    this->actionList->action = nullptr;
    this->actionList->next = nullptr;
}

DynamicJsonDocument Service::prepareMessage(int capacity, String type)
{
    DynamicJsonDocument doc(capacity);
    doc["messageType"] = "serviceInteraction";
    JsonObject data = doc.createNestedObject("data");
    data["serviceId"] = this->id;
    JsonObject dataValue = data.createNestedObject("data");
    dataValue["messageType"] = type;
    return doc;
}

void Service::interpretMessage(HClient &client, Sender *sender, JsonObject &json)
{
    String messageType = json["messageType"];

    if (messageType.equals("requestAction"))
    {

        if (!json.containsKey("data"))
        {
            return;
        }
        JsonObject data = json["data"];
        if (!data.containsKey("name"))
        {
            return;
        }
        String actionName = data["name"];
        Action *action = findActionWithId(actionName);
        if (action == nullptr)
        {
            return;
        }
        if (!action->areArgumentsValid(data))
        {
            return;
        }

        JsonObject actionParameters = data["data"];
        ActionStatus* actionStatus = action->invokeAction(client, actionParameters );
        update(sender);
        action->getHandler()(actionStatus, data);
    }
    else if (messageType.equals("readAllProperties"))
    {
        PropertyNode *propertyNode = this->propertyList;
        while (propertyNode->next != nullptr)
        {
            {
                DynamicJsonDocument doc = this->prepareMessage(1000, "propertyStatus");
                JsonObject data = doc["data"]["data"];
                propertyNode->property->addToJson(data);
                String output;
                serializeJson(doc, output);
                sender->send(output, client);
            }
            propertyNode = propertyNode->next;
        }
    }
}

void Service::addProperty(Property &property)
{
    PropertyNode *newNode = new PropertyNode();
    newNode->property = &property;
    newNode->next = this->propertyList;
    this->propertyList = newNode;
}

void Service::addAction(String id, std::vector<const char *> types, String description, void (*handler)(ActionStatus *actionStatus, JsonObject jsonObject))
{
    ActionNode *newNode = new ActionNode();
    Action *action = new Action(id, types, description);
    action->setHandler(handler);
    newNode->action = action;
    newNode->next = this->actionList;
    this->actionList = newNode;
}

Action *Service::findActionWithId(String id)
{
    ActionNode *actionNode = this->actionList;
    while (actionNode->next != nullptr)
    {
        if (actionNode->action->getId().equals(id))
        {
            return actionNode->action;
        }
        actionNode = actionNode->next;
    }
    return nullptr;
}

void Service::update(Sender *sender)
{

    PropertyNode *propertyNode = this->propertyList;
    while (propertyNode->next != nullptr)
    {
        if (propertyNode->property->isChanged())
        {
            propertyNode->property->setChanged(false);
            DynamicJsonDocument doc = this->prepareMessage(1000, "propertyStatus");
            JsonObject data = doc["data"]["data"].createNestedObject("value");
            propertyNode->property->addToJson(data);
            String output;
            serializeJson(doc, output);
            sender->sendAll(output);
        }
        propertyNode = propertyNode->next;
    }

    ActionNode *actionNode = this->actionList;
    while (actionNode->next != nullptr)
    {
        ActionStatusNode *prevActionStatusNode = actionNode->action->getActionStatusList();
        ActionStatusNode *actionStatusNode = prevActionStatusNode->next;
        while (actionStatusNode != nullptr)
        {
            String actionStatusClientId = actionStatusNode->actionStatus->getClientId();
            HClient *client = nullptr;
            HClient *clients = sender->getClients();
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].isConnected() && clients[i].getId().equals(actionStatusClientId))
                {
                    client = &clients[i];
                    break;
                }
            }

            if (client != nullptr && actionStatusNode->actionStatus->isChanged())
            {
                actionStatusNode->actionStatus->setChanged(false);
                DynamicJsonDocument doc = this->prepareMessage(1000, "actionStatus");
                JsonObject data = doc["data"]["data"]["value"].createNestedObject("data");
                data["id"] = actionStatusNode->actionStatus->getId();
                data["actionId"] = actionStatusNode->actionStatus->getActionId();
                data["requestId"] = actionStatusNode->actionStatus->getRequestId();
                data["message"] = actionStatusNode->actionStatus->getMessage();
                data["userMessage"] = actionStatusNode->actionStatus->getUserMessage();
                String output;
                serializeJson(doc, output);
                sender->send(output, *client);
            }

            if (actionStatusNode->actionStatus->getStatus() != ActionStatus::PENDING)
            {
                prevActionStatusNode->next = actionStatusNode->next;
                delete actionStatusNode->actionStatus;
                delete actionStatusNode;
            }

            prevActionStatusNode = actionStatusNode;
            actionStatusNode = actionStatusNode->next;
        }
        actionNode = actionNode->next;
    }
}
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

void Service::createJSONDescription(JsonObject jsonObject)
{
    jsonObject["id"] = this->id;
    jsonObject["description"] = this->description;
    JsonArray types = jsonObject.createNestedArray("@type");
    for (auto type : this->types)
    {
        types.add(type);
    }

    JsonObject properties = jsonObject.createNestedObject("properties");

    PropertyNode *propertyNode = this->propertyList;
    while (propertyNode->next != nullptr)
    {
        JsonObject property = properties.createNestedObject(propertyNode->property->getId());
        property["id"] = propertyNode->property->getId();
        property["description"] = propertyNode->property->getDescription();
        JsonArray semanticTypesArray = property.createNestedArray("@type");
        for (auto semanticType : propertyNode->property->getSemanticTypes())
        {
            semanticTypesArray.add(semanticType);
        }
        property["type"] = propertyNode->property->getType();
        propertyNode = propertyNode->next;
    }

    JsonObject actions = jsonObject.createNestedObject("actions");

    ActionNode *actionNode = this->actionList;
    while (actionNode->next != nullptr)
    {
        JsonObject action = actions.createNestedObject(actionNode->action->getId());
        action["id"] = actionNode->action->getId();
        action["description"] = actionNode->action->getDescription();
        JsonArray semanticTypesArray = action.createNestedArray("@type");
        for (auto semanticType : actionNode->action->getSemanticTypes())
        {
            semanticTypesArray.add(semanticType);
        }
        actionNode = actionNode->next;
    }
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
        if (!data.containsKey("id"))
        {
            return;
        }
        String actionId = data["id"];
        Action *action = findActionWithId(actionId);
        if (action == nullptr)
        {
            return;
        }
        if (!action->areArgumentsValid(data))
        {
            return;
        }

        JsonVariant actionParameters = data["data"];
        ActionStatus *actionStatus = action->invokeAction(client, actionParameters);
        update(sender);
        action->getHandler()(actionStatus, data);
    }
    else if (messageType.equals("readAllProperties"))
    {
        PropertyNode *propertyNode = this->propertyList;
        while (propertyNode->next != nullptr)
        {
            {
                DynamicJsonDocument doc = this->prepareMessage(PROPERTY_STATUS_JSON_SIZE, "propertyStatus");
                JsonObject data = doc["data"]["data"].createNestedObject("data");
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

void Service::addAction(const char * id, std::vector<const char *> types, const char * description, void (*handler)(ActionStatus *actionStatus, JsonVariant data))
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
        if (id.equals(actionNode->action->getId()))
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
            DynamicJsonDocument doc = this->prepareMessage(PROPERTY_STATUS_JSON_SIZE, "propertyStatus");
            JsonObject data = doc["data"]["data"].createNestedObject("data");
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
                DynamicJsonDocument doc = this->prepareMessage(ACTION_STATUS_JSON_SIZE, "actionStatus");
                JsonObject data = doc["data"]["data"].createNestedObject("data");
                data["id"] = actionStatusNode->actionStatus->getId();
                data["actionId"] = actionStatusNode->actionStatus->getActionId();
                data["requestId"] = actionStatusNode->actionStatus->getRequestId();
                data["status"] = actionStatusNode->actionStatus->getMessage();
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
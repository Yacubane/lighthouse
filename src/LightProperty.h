#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "LightDefines.h"

class Property
{
public:
    Property(const char *id, std::vector<const char *> semanticTypes, const char *description, bool readOnly = true)
    {
        this->id = id;
        this->changed = false;
        this->description = description;
        this->semanticTypes = semanticTypes;
        this->error = true;
        this->errorType = nullptr;
        this->errorMessage = nullptr;
        this->setError("NoValue", "There is no value");
        this->readOnly = readOnly;
        this->onPropertySetHandler = nullptr;
        this->watchable = true;
    }

    bool isReadOnly()
    {
        return this->readOnly;
    }

    bool isWatchable() {
        return this->watchable;
    }

    void setWatchable(bool watchable) {
        this->watchable = watchable;
    }

    bool isChanged()
    {
        return this->changed;
    }

    void setChanged(bool flag)
    {
        this->changed = flag;
    }

    const char *getId()
    {
        return this->id;
    }

    const char *getDescription()
    {
        return this->description;
    }

    void setError(const char *errorType, const char *errorMessage)
    {
        if (!this->error || !String(this->errorType).equals(errorType) || !String(this->errorMessage).equals(errorMessage))
        {
            this->setChanged(true);
        }
        this->error = true;

        if (this->errorType != nullptr)
        {
            delete[] this->errorType;
        }
        if (this->errorMessage != nullptr)
        {
            delete[] this->errorMessage;
        }

        this->errorType = new char[strlen(errorType) + 1];
        this->errorMessage = new char[strlen(errorMessage) + 1];

        strncpy(this->errorType, errorType, strlen(errorType));
        this->errorType[strlen(errorType)] = '\0';
        strncpy(this->errorMessage, errorMessage, strlen(errorMessage));
        this->errorMessage[strlen(errorMessage)] = '\0';
        this->onPropertySet();
    }

    void unsetError()
    {
        if (this->error)
        {
            this->setChanged(true);
        }
        this->error = false;
        this->onPropertySet();
    }

    bool isError()
    {
        return this->error;
    }

    char *getErrorType()
    {
        return this->errorType;
    }

    char *getErrorMessage()
    {
        return this->errorMessage;
    }

    std::vector<const char *> getSemanticTypes()
    {
        return this->semanticTypes;
    }

    void setOnPropertySetHandler(void (*onPropertySetHandler)())
    {
        this->onPropertySetHandler = onPropertySetHandler;
    }

    void onPropertySet() {
        if (this->onPropertySetHandler != nullptr)
        {
            this->onPropertySetHandler();
        }
    }

    virtual String getType() = 0;
    virtual void addToJson(JsonObject jsonObject) = 0;
    virtual bool setValue(JsonVariant jsonVariant) = 0;

protected:
    bool error;
    char *errorType;
    char *errorMessage;

private:
    const char *id;
    const char *description;
    bool changed;
    std::vector<const char *> semanticTypes;
    void (*onPropertySetHandler)();
    bool readOnly;
    bool watchable;
};

class BooleanProperty : public Property
{
public:
    BooleanProperty(const char *id, std::vector<const char *> semanticTypes, const char *description, bool isReadOnly = true) : Property(id, semanticTypes, description, isReadOnly) {}

    void setValue(bool value)
    {
        if (this->value != value)
        {
            this->setChanged(true);
        }
        this->value = value;
        this->error = false;
        this->onPropertySet();
    }

    bool setValue(JsonVariant jsonValue)
    {
        if (jsonValue.isNull() || !jsonValue.is<bool>())
        {
            return false;
        }
        bool value = jsonValue.as<bool>();
        this->setValue(value);
        return true;
    }

    bool getValue()
    {
        return value;
    }

    String getType()
    {
        return "boolean";
    }

    void addToJson(JsonObject jsonObject)
    {
        jsonObject["id"] = this->getId();
        jsonObject["error"] = this->error;
        if (this->error)
        {
            jsonObject["value"] = nullptr;
            jsonObject["errorType"] = this->errorType;
            jsonObject["errorMessage"] = this->errorMessage;
        }
        else
        {
            jsonObject["value"] = this->getValue();
        }
    }

private:
    bool value;
};

class IntegerProperty : public Property
{
public:
    IntegerProperty(const char *id, std::vector<const char *> semanticTypes, const char *description, bool isReadOnly = true) : Property(id, semanticTypes, description, isReadOnly) {}

    void setValue(int32_t value)
    {
        if (this->value != value)
        {
            this->setChanged(true);
        }
        this->value = value;
        this->error = false;
        this->onPropertySet();
    }

    bool setValue(JsonVariant jsonValue)
    {
        if (jsonValue.isNull() || !jsonValue.is<int>())
        {
            return false;
        }
        int value = jsonValue.as<int>();
        this->setValue(value);
        return true;
    }

    int32_t getValue()
    {
        return value;
    }

    String getType()
    {
        return "integer";
    }

    void addToJson(JsonObject jsonObject)
    {
        jsonObject["id"] = this->getId();
        jsonObject["error"] = this->error;
        if (this->error)
        {
            jsonObject["value"] = nullptr;
            jsonObject["errorType"] = this->errorType;
            jsonObject["errorMessage"] = this->errorMessage;
        }
        else
        {
            jsonObject["value"] = this->getValue();
        }
    }

private:
    int32_t value;
};

class StringProperty : public Property
{
public:
    StringProperty(const char *id, std::vector<const char *> semanticTypes, const char *description, bool isReadOnly = true) : Property(id, semanticTypes, description, isReadOnly) {}

    void setValue(String value)
    {
        if (!value.equals(this->value))
        {
            this->setChanged(true);
        }
        this->value = value;
        this->error = false;
        this->onPropertySet();
    }

    bool setValue(JsonVariant jsonValue)
    {
        if (jsonValue.isNull() || !jsonValue.is<const char *>())
        {
            return false;
        }
        const char *value = jsonValue.as<const char *>();
        this->setValue(value);
        return true;
    }

    String getValue()
    {
        return value;
    }

    String getType()
    {
        return "string";
    }

    void addToJson(JsonObject jsonObject)
    {
        jsonObject["id"] = this->getId();
        jsonObject["error"] = this->error;
        if (this->error)
        {
            jsonObject["value"] = nullptr;
            jsonObject["errorType"] = this->errorType;
            jsonObject["errorMessage"] = this->errorMessage;
        }
        else
        {
            jsonObject["value"] = this->getValue();
        }
    }

private:
    String value;
};

class NumberProperty : public Property
{
public:
    NumberProperty(const char *id, std::vector<const char *> semanticTypes, const char *description, bool isReadOnly = true) : Property(id, semanticTypes, description, isReadOnly) {}

    void setValue(double value)
    {
        if (this->value != value)
        {
            this->setChanged(true);
        }
        this->value = value;
        this->error = false;
        this->onPropertySet();
    }

    bool setValue(JsonVariant jsonValue)
    {
        if (jsonValue.isNull() || !jsonValue.is<double>())
        {
            return false;
        }
        double value = jsonValue.as<double>();
        this->setValue(value);
        return true;
    }

    double getValue()
    {
        return value;
    }

    String getType()
    {
        return "number";
    }

    void addToJson(JsonObject jsonObject)
    {
        jsonObject["id"] = this->getId();
        jsonObject["error"] = this->error;
        if (this->error)
        {
            jsonObject["value"] = nullptr;
            jsonObject["errorType"] = this->errorType;
            jsonObject["errorMessage"] = this->errorMessage;
        }
        else
        {
            jsonObject["value"] = this->getValue();
        }
    }

private:
    double value;
};
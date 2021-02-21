#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "LightDefines.h"

class Property
{
public:
    Property(const char *id, std::vector<const char *> semanticTypes, const char *description)
    {
        this->id = id;
        this->changed = false;
        this->description = description;
        this->semanticTypes = semanticTypes;
        this->error = true;
        this->errorType = "NoValue";
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

    void setError(char *errorType, char *errorMessage)
    {
        if (!this->error)
        {
            this->setChanged(true);
        }
        this->error = true;
        this->errorType = errorType;
        this->errorMessage = errorMessage;
    }

    void unsetError()
    {
        if (this->error)
        {
            this->setChanged(true);
        }
        this->error = false;
    }

    bool isError()
    {
        return this->error;
    }

    char *getErrorType()
    {
        return this->errorType;
    }

    char *getErrorType()
    {
        return this->errorMessage;
    }

    std::vector<const char *> getSemanticTypes()
    {
        return this->semanticTypes;
    }

    virtual String getType() = 0;
    virtual void addToJson(JsonObject jsonObject) = 0;

protected:
    bool error;
    char *errorType;
    char *errorMessage;

private:
    const char *id;
    const char *description;
    bool changed;
    std::vector<const char *> semanticTypes;
};

class BooleanProperty : public Property
{
public:
    BooleanProperty(const char *id, std::vector<const char *> semanticTypes, const char *description) : Property(id, semanticTypes, description)
    {
    }

    void setValue(bool value)
    {
        if (this->value != value)
        {
            this->setChanged(true);
        }
        this->value = value;
        this->error = false;
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
    IntegerProperty(const char *id, std::vector<const char *> semanticTypes, const char *description) : Property(id, semanticTypes, description) {}

    void setValue(int32_t value)
    {
        if (this->value != value)
        {
            this->setChanged(true);
        }
        this->value = value;
        this->error = false;
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
    StringProperty(const char *id, std::vector<const char *> semanticTypes, const char *description) : Property(id, semanticTypes, description) {}

    void setValue(String value)
    {
        if (!value.equals(this->value))
        {
            this->setChanged(true);
        }
        this->value = value;
        this->error = false;
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
    NumberProperty(const char *id, std::vector<const char *> semanticTypes, const char *description) : Property(id, semanticTypes, description) {}

    void setValue(double value)
    {
        if (this->value != value)
        {
            this->setChanged(true);
        }
        this->value = value;
        this->error = false;
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
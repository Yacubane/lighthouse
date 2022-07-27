#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "LightDefines.h"

class Property
{
public:
    Property(const char *id, std::vector<const char *> semanticTypes, const char *description, bool readOnly = true, bool informOnChange = true)
    {
        this->id = id;
        this->changed = false;
        this->description = description;
        this->semanticTypes = semanticTypes;
        this->error = true;
        this->errorType = nullptr;
        this->errorMessage = nullptr;
        this->onPropertySetHandler = nullptr;
        this->setError("NoValue", "There is no value");
        this->readOnly = readOnly;
        this->informOnChange = informOnChange;
    }

    bool isReadOnly()
    {
        return this->readOnly;
    }

    bool isInformOnChange()
    {
        return this->informOnChange;
    }

    void setInformOnChange(bool informOnChange)
    {
        this->informOnChange = informOnChange;
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

    void onPropertySet()
    {
        this->lastTimeSetMillis = millis();
        if (this->onPropertySetHandler != nullptr)
        {
            this->onPropertySetHandler();
        }
    }

    unsigned long getLastTimeSetMillis()
    {
        return this->lastTimeSetMillis;
    }

    virtual String getType() = 0;
    virtual void addToJson(JsonObject jsonObject) = 0;
    virtual bool setValue(JsonVariant jsonVariant) = 0;

protected:
    bool error;
    char *errorType;
    char *errorMessage;

private:
    unsigned long lastTimeSetMillis = 0;
    const char *id;
    const char *description;
    bool changed;
    std::vector<const char *> semanticTypes;
    void (*onPropertySetHandler)();
    bool readOnly;
    bool informOnChange;
};

template <typename U, typename V, char const *type>
class TemplatedProperty : public Property
{
    using Property::Property;

public:
    void setValue(U value)
    {
        if (!isValueValid(value)) {
            setError("WrongValue", "Value is not valid (it can be for example empty)");
            return;
        }
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
        if (jsonValue.isNull() || !jsonValue.is<V>())
        {
            return false;
        }
        U value = jsonValue.as<V>();
        this->setValue(value);
        return true;
    }

    U getValue()
    {
        if (onGetValueHandler != nullptr)
        {
            return onGetValueHandler();
        }
        return value;
    }

    String getType()
    {
        return type;
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

    void setOnGetValueHandler(U (*onGetValueHandler)())
    {
        this->onGetValueHandler = onGetValueHandler;
        this->unsetError();
    }

    virtual bool isValueValid(U value) {
        return true;
    }

    private:
        U (*onGetValueHandler)() = nullptr;
        U value;

};

static const char BooleanPropertyType[] = "boolean";
class BooleanProperty : public TemplatedProperty<bool, bool, BooleanPropertyType>
{
    using TemplatedProperty::TemplatedProperty;
};

static const char IntegerPropertyType[] = "integer";
class IntegerProperty : public TemplatedProperty<int64_t, int64_t, IntegerPropertyType>
{
    using TemplatedProperty::TemplatedProperty;
    public:
    bool isValueValid(int64_t value) override {
        if (isnan(value)) {
            return false;
        }
        return true;
    }
};

static const char StringPropertyType[] = "string";
class StringProperty : public TemplatedProperty<String, const char *, StringPropertyType>
{
    using TemplatedProperty::TemplatedProperty;
};

static const char NumberPropertyType[] = "number";
class NumberProperty : public TemplatedProperty<double, double, NumberPropertyType>
{
    using TemplatedProperty::TemplatedProperty;
    public:
    bool isValueValid(double value) override {
        if (isnan(value)) {
            return false;
        }
        return true;
    }
};

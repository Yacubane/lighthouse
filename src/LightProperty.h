#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "LightDefines.h"

class Property
{
public:
    enum SendingUpdatesStrategy
    {
        NEVER, // never update
        MANUAL, // update only manually
        OPTIMIZE, // with maximum period + on change
        ON_CHANGE, // update on change
        ALWAYS, // always update
    };
    Property(const char *id, std::vector<const char *> semanticTypes = {}, const char *description = "", bool readOnly = true)
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
    }

    void setReadOnly(bool readOnly) 
    {
        this->readOnly = readOnly;
    }

    bool isReadOnly()
    {
        return this->readOnly;
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

    void setLastTimeSentUpdateMillis(unsigned long millis)
    {
        this->lastTimeSentUpdateMillis = millis;
    }

    unsigned long getLastTimeSentUpdateMillis()
    {
        return this->lastTimeSentUpdateMillis;
    }

    void setMinPeriodForSendingUpdates(unsigned long millis)
    {
        this->minPeriodForSendingUpdates = millis;
    }

    bool shouldSendUpdate() {
        switch (sendingUpdatesStrategy) {
            case SendingUpdatesStrategy::NEVER:
                return false;
            case SendingUpdatesStrategy::MANUAL:
                return this->shouldUpdatesBeSent;
            case SendingUpdatesStrategy::OPTIMIZE:
                return this->isChanged() && (millis() - this->lastTimeSentUpdateMillis > minPeriodForSendingUpdates);
            case SendingUpdatesStrategy::ON_CHANGE:
                return this->isChanged();
            case SendingUpdatesStrategy::ALWAYS:
                return true;;
        }
    }

    void afterSendingUpdates() {
        this->setChanged(false);
        this->shouldUpdatesBeSent = false;
        this->lastTimeSentUpdateMillis = millis();
    }

    void setSendingUpdatesStrategy(enum SendingUpdatesStrategy sendingUpdatesStrategy) {
        this->sendingUpdatesStrategy = sendingUpdatesStrategy;
    }

    void setSendUpdateFlag() {
        this->shouldUpdatesBeSent = true;
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
    unsigned long lastTimeSentUpdateMillis = 0;
    unsigned long minPeriodForSendingUpdates = LIGHTHOUSE_DEFAULT_MIN_PROPERTY_UPDATES_PERIOD_MS;
    const char *id;
    const char *description;
    bool changed;
    enum SendingUpdatesStrategy sendingUpdatesStrategy = SendingUpdatesStrategy::OPTIMIZE;
    std::vector<const char *> semanticTypes;
    void (*onPropertySetHandler)();
    bool readOnly;
    bool shouldUpdatesBeSent = false;
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

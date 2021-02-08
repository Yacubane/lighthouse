#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "LightDefines.h"

class Property
{
public:
    Property(const char *name, std::vector<const char *> semanticTypes, const char *description)
    {
        this->name = name;
        this->changed = false;
        this->description = description;
        this->semanticTypes = semanticTypes;
    }

    bool isChanged()
    {
        return this->changed;
    }

    void setChanged(bool flag)
    {
        this->changed = flag;
    }

    const char *getName()
    {
        return this->name;
    }

    const char *getDescription()
    {
        return this->description;
    }

    std::vector<const char *> getSemanticTypes()
    {
        return this->semanticTypes;
    }

    virtual String getType() = 0;
    virtual void addToJson(JsonObject jsonObject) = 0;

private:
    const char *name;
    const char *description;
    boolean changed;
    std::vector<const char *> semanticTypes;
};

class BooleanProperty : public Property
{
public:
    BooleanProperty(const char * name, std::vector<const char *> semanticTypes, const char *description) : Property(name, semanticTypes, description)
    {
    }

    void setValue(bool value)
    {
        this->value = value;
        this->setChanged(true);
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
        jsonObject["name"] = this->getName();
        jsonObject["value"] = this->getValue();
    }

private:
    bool value;
};

class IntegerProperty : public Property
{
public:
    IntegerProperty(const char * name, std::vector<const char *> semanticTypes, const char *description) : Property(name, semanticTypes, description) {}

    void setValue(int32_t value)
    {
        this->value = value;
        this->setChanged(true);
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
        jsonObject["name"] = this->getName();
        jsonObject["value"] = this->getValue();
    }

private:
    int32_t value;
};

class StringProperty : public Property
{
public:
    StringProperty(const char * name, std::vector<const char *> semanticTypes, const char *description) : Property(name, semanticTypes, description) {}

    void setValue(String value)
    {
        this->value = value;
        this->setChanged(true);
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
        jsonObject["name"] = this->getName();
        jsonObject["value"] = this->getValue();
    }

private:
    String value;
};

class FloatProperty : public Property
{
public:
    FloatProperty(const char * name, std::vector<const char *> semanticTypes, const char *description) : Property(name, semanticTypes, description) {}

    void setValue(double value)
    {
        this->value = value;
        this->setChanged(true);
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
        jsonObject["name"] = this->getName();
        jsonObject["value"] = this->getValue();
    }

private:
    double value;
};
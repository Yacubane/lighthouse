#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class Property {
public:
    Property(String name) {
        this->name = name;
        this->changed = false;
    }

    bool isChanged() {
        return this->changed;
    }

    void setChanged(bool flag) {
        this->changed = flag;
    }

    String getName() {
        return this->name;
    }

    virtual void addToJson(JsonObject jsonObject) = 0;
private:
    String name;
    boolean changed;
};

class BooleanProperty : public Property {
public:
    BooleanProperty(String name) : Property(name) {

    }

    void setValue(bool value) {
        this->value = value;
        this->setChanged(true);
    }

    bool getValue() {
        return value;
    }

    void addToJson(JsonObject jsonObject) {
        jsonObject["name"] = this->getName();
        jsonObject["value"] = this->getValue();
    }
private:
    bool value;
};

class IntegerProperty : public Property {
public:
    IntegerProperty(String name) : Property(name) { }

    void setValue(int32_t value) {
        this->value = value;
        this->setChanged(true);
    }

    int32_t getValue() {
        return value;
    }

    void addToJson(JsonObject jsonObject) {
        jsonObject["name"] = this->getName();
        jsonObject["value"] = this->getValue();
    }
private:
    int32_t value;
};

class StringProperty : public Property {
public:
    StringProperty(String name) : Property(name) { }

    void setValue(String value) {
        this->value = value;
        this->setChanged(true);
    }

    String getValue() {
        return value;
    }

    void addToJson(JsonObject jsonObject) {
        jsonObject["name"] = this->getName();
        jsonObject["value"] = this->getValue();
    }
private:
    String value;
};

class FloatProperty : public Property {
public:
    FloatProperty(String name) : Property(name) { }

    void setValue(double value) {
        this->value = value;
        this->setChanged(true);
    }

    double getValue() {
        return value;
    }

    void addToJson(JsonObject jsonObject) {
        jsonObject["name"] = this->getName();
        jsonObject["value"] = this->getValue();
    }
private:
    double value;
};
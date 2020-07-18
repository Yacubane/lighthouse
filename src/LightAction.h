#pragma once

#include <Arduino.h>
#include "LightProperty.h"

class Action {
public:
    Action(String name) { 
        this->name = name;
    }

    String getName() { 
        return this->name; 
    }

    void setSimpleHandler(void(*simpleHandler)()) { 
        this->simpleHandler = simpleHandler; 
    }

    bool areArgumentsValid(JsonObject data) { 
        return true; 
    }

    void invokeAction(JsonObject data) { 
        this->simpleHandler(); 
    }

private:
    String name;
    void (*simpleHandler)();
};
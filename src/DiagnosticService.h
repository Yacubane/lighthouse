#pragma once
#include <Arduino.h>
#include "LighthouseImports.h"
#include <ArduinoJson.h>
#include "LightService.h"
#include "LightProperty.h"


#if defined(ESP8266)
class DiagnosticService : public Service
{
public:
    DiagnosticService() : Service("diagnostic-service", {"DiagnosticService"}, ""),
                          mac("mac", {}, "", true, false),
                          fullVersion("fullVersion", {}, "", true, false),
                          sdkVersion("sdkVersion", {}, "", true, false),
                          sketchSize("sketchSize", {}, "", true, false),
                          freeSketchSpace("freeSketchSpace", {}, "", true, false),
                          sketchMD5("sketchMD5", {}, "", true, false),
                          chipId("chipId", {}, "", true, false),
                          coreVersion("coreVersion", {}, "", true, false),
                          bootMode("bootMode", {}, "", true, false),
                          bootVersion("bootVersion", {}, "", true, false),
                          rssi("RSSI", {}, "", true, false),
                          vcc("vcc", {}, "", true, false),
                          cycleCount("cycleCount", {}, "", true, false),
                          freeHeap("freeHeap", {}, "", true, false),
                          millisProperty("millis", {}, "", true, false)
    {
        this->mac.setValue(WiFi.macAddress());
        this->fullVersion.setValue(ESP.getFullVersion());
        this->sdkVersion.setValue(ESP.getSdkVersion());
        this->sketchSize.setValue(ESP.getSketchSize());
        this->freeSketchSpace.setValue(ESP.getFreeSketchSpace());
        this->sketchMD5.setValue(ESP.getSketchMD5());
        this->chipId.setValue(ESP.getChipId());
        this->coreVersion.setValue(ESP.getCoreVersion());
        this->bootMode.setValue(ESP.getBootMode());
        this->bootVersion.setValue(ESP.getBootVersion());

        this->addProperty(mac);
        this->addProperty(fullVersion);
        this->addProperty(sdkVersion);
        this->addProperty(sketchSize);
        this->addProperty(freeSketchSpace);
        this->addProperty(sketchMD5);
        this->addProperty(chipId);
        this->addProperty(coreVersion);
        this->addProperty(bootMode);
        this->addProperty(bootVersion);
        this->addProperty(rssi);
        this->addProperty(vcc);
        this->addProperty(cycleCount);
        this->addProperty(freeHeap);
        this->addProperty(millisProperty);
    }

    void update()
    {
        this->rssi.setValue(WiFi.RSSI());
        this->vcc.setValue(ESP.getVcc());
        this->cycleCount.setValue(ESP.getCycleCount());
        this->freeHeap.setValue(ESP.getFreeHeap());
        this->millisProperty.setValue(millis());
    }

private:
    StringProperty mac;
    StringProperty fullVersion;
    StringProperty sdkVersion;
    IntegerProperty sketchSize;
    IntegerProperty freeSketchSpace;
    StringProperty sketchMD5;
    IntegerProperty chipId;
    StringProperty coreVersion;
    IntegerProperty bootMode;
    IntegerProperty bootVersion;
    IntegerProperty rssi;
    IntegerProperty vcc;
    IntegerProperty cycleCount;
    IntegerProperty freeHeap;
    IntegerProperty millisProperty;
};
#elif defined(ESP32)
class DiagnosticService : public Service
{
public:
    DiagnosticService() : Service("diagnostic-service", {"DiagnosticService"}, ""),
                          mac("mac", {}, "", true, false),
                          sdkVersion("sdkVersion", {}, "", true, false),
                          sketchSize("sketchSize", {}, "", true, false),
                          freeSketchSpace("freeSketchSpace", {}, "", true, false),
                          sketchMD5("sketchMD5", {}, "", true, false),
                          cycleCount("cycleCount", {}, "", true, false),
                          freeHeap("freeHeap", {}, "", true, false),
                          millisProperty("millis", {}, "", true, false)
    {
        this->mac.setValue(WiFi.macAddress());
        this->sdkVersion.setValue(ESP.getSdkVersion());
        this->sketchSize.setValue(ESP.getSketchSize());
        this->freeSketchSpace.setValue(ESP.getFreeSketchSpace());
        this->sketchMD5.setValue(ESP.getSketchMD5());

        this->addProperty(mac);
        this->addProperty(sdkVersion);
        this->addProperty(sketchSize);
        this->addProperty(freeSketchSpace);
        this->addProperty(sketchMD5);
        this->addProperty(cycleCount);
        this->addProperty(freeHeap);
        this->addProperty(millisProperty);
    }

    void update()
    {
        this->cycleCount.setValue(ESP.getCycleCount());
        this->freeHeap.setValue(ESP.getFreeHeap());
        this->millisProperty.setValue(millis());
    }

private:
    StringProperty mac;
    StringProperty sdkVersion;
    IntegerProperty sketchSize;
    IntegerProperty freeSketchSpace;
    StringProperty sketchMD5;
    IntegerProperty cycleCount;
    IntegerProperty freeHeap;
    IntegerProperty millisProperty;
};
#endif
#pragma once
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "LightService.h"
#include "LightProperty.h"

class DiagnosticService : public Service
{
public:
    DiagnosticService() : Service("diagnostic-service", {"DiagnosticService"}, ""),
                          mac("mac", {}, ""),
                          fullVersion("fullVersion", {}, ""),
                          sdkVersion("sdkVersion", {}, ""),
                          sketchSize("sketchSize", {}, ""),
                          freeSketchSpace("freeSketchSpace", {}, ""),
                          sketchMD5("sketchMD5", {}, ""),
                          chipId("chipId", {}, ""),
                          coreVersion("coreVersion", {}, ""),
                          bootMode("bootMode", {}, ""),
                          bootVersion("bootVersion", {}, ""),
                          rssi("RSSI", {}, ""),
                          vcc("vcc", {}, ""),
                          bssid("BSSID", {}, ""),
                          ssid("SSID", {}, ""),
                          cycleCount("cycleCount", {}, ""),
                          freeHeap("freeHeap", {}, ""),
                          millisProperty("millis", {}, "")
    {
        mac.setWatchable(false);
        fullVersion.setWatchable(false);
        sdkVersion.setWatchable(false);
        sketchSize.setWatchable(false);
        freeSketchSpace.setWatchable(false);
        sketchMD5.setWatchable(false);
        chipId.setWatchable(false);
        coreVersion.setWatchable(false);
        bootMode.setWatchable(false);
        bootVersion.setWatchable(false);
        rssi.setWatchable(false);
        vcc.setWatchable(false);
        bssid.setWatchable(false);
        rssi.setWatchable(false);
        ssid.setWatchable(false);
        cycleCount.setWatchable(false);
        freeHeap.setWatchable(false);
        millisProperty.setWatchable(false);

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
        this->addProperty(bssid);
        this->addProperty(ssid);
        this->addProperty(cycleCount);
        this->addProperty(freeHeap);
        this->addProperty(millisProperty);
    }

    void update()
    {
        this->rssi.setValue(WiFi.RSSI());
        this->vcc.setValue(ESP.getVcc());
        this->bssid.setValue(WiFi.BSSIDstr());
        this->ssid.setValue(WiFi.SSID());
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
    StringProperty bssid;
    StringProperty ssid;
    IntegerProperty cycleCount;
    IntegerProperty freeHeap;
    IntegerProperty millisProperty;
};
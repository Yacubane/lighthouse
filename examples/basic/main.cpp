#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Lighthouse.h"

// Change these values
#define WIFI_SSID "TV_ROOM_2.4G"
#define WIFI_PASSWORD "..."

Device thing("home-device", 8123);

Service gateWicketService("wicket-gate", {"Button", "WicketGate"}, "Gate opener/closer");

Service gateService("gate", {"ToggleButton", "Gate"}, "Gate opener/closer");
StringProperty gateProperty("gate", {"OpenProperty"}, "");

Service lightService("outside-lights", {"ToggleButton", "Light"}, "Light service");
BooleanProperty lightProperty("outside-lights", {"OnOffProperty"}, "");

Service temperatureService("room-temperature", {"TemperatureSensor", "Sensor"}, "Temperature in room");
NumberProperty temperatureProperty("room-temperature", {"NumberProperty"}, "");

Service humidityService("room-humidity", {"HumiditySensor", "Sensor"}, "Humidity in room");
NumberProperty humidityProperty("room-humidity", {"NumberProperty"}, "");

void toggleWicketGateHandler(ActionStatus *actionStatus, JsonVariant data)
{
    actionStatus->set(ActionStatus::COMPLETED, "COMPLETED", "Action completed successfully");
}

void toggleGateHandler(ActionStatus *actionStatus, JsonVariant data)
{
    gateProperty.setValue(gateProperty.getValue().equals("OPEN") ? "CLOSE" : "OPEN");
    actionStatus->set(ActionStatus::COMPLETED, "COMPLETED", "Action completed successfully");
}

void toggleLightHandler(ActionStatus *actionStatus, JsonVariant data)
{
    lightProperty.setValue(lightProperty.getValue() ? false : true);
    actionStatus->set(ActionStatus::COMPLETED, "COMPLETED", "Action completed successfully");
}

void setup()
{
    Serial.begin(9600);

    thing.setWiFi(WIFI_SSID, WIFI_PASSWORD);
    thing.setOTA("123456");
    thing.setPassword("123456");

    gateWicketService.addAction("toggle", {"Toggle"}, "Toggle wicket gate action", toggleWicketGateHandler);
    thing.addService(&gateWicketService);

    gateService.addProperty(gateProperty);
    gateService.addAction("toggle", {"Toggle"}, "Toggle gate action", toggleGateHandler);
    thing.addService(&gateService);

    lightService.addProperty(lightProperty);
    lightService.addAction("toggle", {"Toggle"}, "Toggle light action", toggleLightHandler);
    thing.addService(&lightService);

    temperatureService.addProperty(temperatureProperty);
    thing.addService(&temperatureService);

    humidityService.addProperty(humidityProperty);
    thing.addService(&humidityService);

    lightProperty.setValue(false);
    gateProperty.setValue("OPEN");
    temperatureProperty.setValue(22);
    humidityProperty.setValue(50);

    thing.start();
}

void loop()
{
    thing.update();
    delay(10);
}

// Example API usage:
/*
var socket = new WebSocket('ws://192.168.100.21:8123');

socket.addEventListener('open', function (event) {
    socket.send('{ "messageType": "authenticate", "data": {"password": "123456"}} { "messageType": "ping" } { "messageType": "describe" } { "messageType": "serviceInteraction", "data": { "serviceId" : "gate", "data": { "messageType": "readAllProperties"}}} { "messageType": "serviceInteraction", "data": { "serviceId" : "gate", "data": { "messageType": "requestAction", "data": { "id": "toggle", "data": { "requestId": "req"}}}}} { "messageType": "keepalive" }');
});

socket.addEventListener('message', function (event) {
    console.log('Message from server ', event.data);
});
*/
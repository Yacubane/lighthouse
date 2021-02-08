#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Lighthouse.h"

// Change these values
#define WIFI_SSID "TV_ROOM_2.4G"
#define WIFI_PASSWORD "..."

Device thing("home-device");
BooleanProperty lightProperty("garden-lights", {"OnOffProperty"}, "");
StringProperty garageDoorsProperty("garage-doors", {"OpenCloseProperty"}, "");

void toggleGarageDoorsHandler(ActionStatus *actionStatus, JsonObject data)
{
  garageDoorsProperty.setValue(garageDoorsProperty.getValue().equals("OPEN") ? "CLOSE" : "OPEN");
  actionStatus->set(ActionStatus::COMPLETED, "COMPLETED", "Action completed successfully");
}

void toggleLightHandler(ActionStatus *actionStatus, JsonObject data)
{
  lightProperty.setValue(lightProperty.getValue() ? false : true);
  actionStatus->set(ActionStatus::COMPLETED, "COMPLETED", "Action completed successfully");
}

void setup()
{
  Serial.begin(9600);

  thing.setup(WIFI_SSID, WIFI_PASSWORD, 8123);
  thing.setPassword("123456");

  Service *garageDoorsService = new Service("gate", {"ToggleButton", "GarageDoors"}, "Garage doors service");
  garageDoorsService->addProperty(garageDoorsProperty);
  garageDoorsService->addAction("toggle", {"Toggle"}, "Toggle garage doors action", toggleGarageDoorsHandler);
  thing.addService(garageDoorsService);

  Service *lightService = new Service("light", {"ToggleButton", "Light"}, "Light service");
  lightService->addProperty(lightProperty);
  lightService->addAction("toggle", {"Toggle"}, "Toggle light action", toggleLightHandler);
  thing.addService(lightService);

  lightProperty.setValue(false);
  garageDoorsProperty.setValue("OPEN");

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
    socket.send('{ "messageType": "authenticate", "data": {"password": "123456"}} { "messageType": "ping" } { "messageType": "describe" } { "messageType": "serviceInteraction", "data": { "serviceId" : "gate", "data": { "messageType": "readAllProperties"}}} { "messageType": "serviceInteraction", "data": { "serviceId" : "gate", "data": { "messageType": "requestAction", "data": { "name": "toggle", "data": { "requestId": "req"}}}}} { "messageType": "keepalive" }');
});

socket.addEventListener('message', function (event) {
    console.log('Message from server ', event.data);
});
*/
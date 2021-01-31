#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Lighthouse.h"

// Change these values
#define WIFI_SSID "TV_ROOM_2.4G"
#define WIFI_PASSWORD "..."

Device thing("home-device");
BooleanProperty lightProperty("garden-lights");
StringProperty garageDoorsProperty("garage-doors");

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
// nc -v <ip_printed_by_esp8266> 8123
// { "messageType": "authenticate", "data": {"password": "123456"}} { "messageType": "ping" } { "messageType": "serviceInteraction", "data": { "serviceId" : "gate", "data": { "messageType": "readAllProperties"}}} { "messageType": "serviceInteraction", "data": { "serviceId" : "gate", "data": { "messageType": "requestAction", "data": { "name": "toggle", "data": { "requestId": "req"}}}}} { "messageType": "keepalive" }
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Lighthouse.h"

// Change these values
#define WIFI_SSID "..."
#define WIFI_PASSWORD "..."

Device device("home-device");
FloatProperty temperature("temperature");

void heatUpHandler() {
  device.broadcastEvent("heatingUp");
  temperature.setValue(temperature.getValue() + 1.0);
}

void coolDownHandler() {
  device.broadcastEvent("coolingDown");
  temperature.setValue(temperature.getValue() - 1.0);
}

void setup() {
  Serial.begin(9600);

  device.setup(WIFI_SSID, WIFI_PASSWORD, 8123);
  device.setPassword("123456");
  device.addProperty(temperature);
  device.addAction("heatUp", heatUpHandler);
  device.addAction("coolDown", coolDownHandler);

  temperature.setValue(22);

  device.start();
}

void loop() {
  device.update();
}

// Example API usage:
// nc -v <ip_printed_by_esp8266> 8123
// {"messageType":"handshake", "data": {"password": "123456"}}{"messageType":"readAllProperties"}{"messageType":"subscribeEverything"}{"messageType":"requestAction", "data": {"name": "heatUp"}}{"messageType":"keepalive"}
#include <Arduino.h>

#include "LedController.h"
#include "Logger.h"
#include "MQTTManager.h"
#include "PositionTracker.h"
#include "RemoteController.h"
#include "WiFiManager.h"

LedController leds;
RemoteController remote;
PositionTracker position;
WiFiManager wifi;
MQTTManager mqtt;

void setup()
{
  Logger::begin();
  Logger::info("3T HSF1 Bridge starting");

  leds.begin();
  remote.begin();
  position.begin();

  wifi.begin();
  mqtt.begin();
}

void loop()
{
  wifi.update();
  mqtt.update();
  position.update();

  leds.setWifi(wifi.isConnected());
  leds.setMqtt(mqtt.isConnected());
}
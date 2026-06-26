#include <Arduino.h>

#include "LedController.h"
#include "Logger.h"
#include "PositionTracker.h"
#include "RemoteController.h"
#include "WiFiManager.h"

LedController leds;
RemoteController remote;
PositionTracker position;
WiFiManager wifi;

void setup()
{
  Logger::begin();
  Logger::info("3T HSF1 Bridge starting");

  leds.begin();
  remote.begin();
  position.begin();
  wifi.begin();
}

void loop()
{
  wifi.update();
  position.update();

  leds.setWifi(wifi.isConnected());
}
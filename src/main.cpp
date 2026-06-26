#include <Arduino.h>
#include "RemoteController.h"
#include "Logger.h"

RemoteController remote;

void setup()
{
  Logger::begin();
  Logger::info("3T HSF1 Bridge starting");

  // For testing
  Logger::info("LED controller initialized");
  Logger::info("Remote controller initialized");
  Logger::debug("Running startup self-test");

  delay(1000);

  Serial.println("3T HSF1 Bridge starting");
  remote.begin();

  Serial.println("Test: Open");
  remote.pressOpen();

  delay(1000);

  Serial.println("Test: Stop");
  remote.pressStop();

  delay(1000);

  Serial.println("Test: Close");
  remote.pressClose();
}

void loop()
{
}
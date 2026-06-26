#include <Arduino.h>
#include "RemoteController.h"

RemoteController remote;

void setup()
{
  Serial.begin(115200);
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
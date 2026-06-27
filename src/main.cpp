#include <Arduino.h>

#include "Application.h"

Application app;

void setup()
{
  app.begin();
}

void loop()
{
  app.update();
  delay(100);
}
#include "LedController.h"

#include <Arduino.h>

#include "Pins.h"

void LedController::begin()
{
    pinMode(PIN_LED_WIFI, OUTPUT);
    pinMode(PIN_LED_MQTT, OUTPUT);
    pinMode(PIN_LED_SEND, OUTPUT);
    pinMode(PIN_LED_ERROR, OUTPUT);

    digitalWrite(PIN_LED_WIFI, LOW);
    digitalWrite(PIN_LED_MQTT, LOW);
    digitalWrite(PIN_LED_SEND, LOW);
    digitalWrite(PIN_LED_ERROR, LOW);
}

void LedController::setWifi(bool connected)
{
    wifiState = connected;
    digitalWrite(PIN_LED_WIFI, connected);
}

void LedController::setMqtt(bool connected)
{
    mqttState = connected;
    digitalWrite(PIN_LED_MQTT, connected);
}

void LedController::setError(bool error)
{
    errorState = error;
    digitalWrite(PIN_LED_ERROR, error);
}

void LedController::flashSend()
{
    digitalWrite(PIN_LED_SEND, HIGH);
    delay(100);
    digitalWrite(PIN_LED_SEND, LOW);
}
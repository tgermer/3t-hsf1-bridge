#include "LedController.h"

#include <Arduino.h>

#include "Pins.h"

void LedController::begin()
{
    pinMode(Pins::Led::WiFi, OUTPUT);
    pinMode(Pins::Led::MQTT, OUTPUT);
    pinMode(Pins::Led::Send, OUTPUT);
    pinMode(Pins::Led::Error, OUTPUT);

    digitalWrite(Pins::Led::WiFi, LOW);
    digitalWrite(Pins::Led::MQTT, LOW);
    digitalWrite(Pins::Led::Send, LOW);
    digitalWrite(Pins::Led::Error, LOW);
}

void LedController::update()
{
    if (!sendLedActive)
    {
        return;
    }

    if (millis() - sendLedStartedAt >= SendFlashDurationMs)
    {
        digitalWrite(Pins::Led::Send, LOW);
        sendLedActive = false;
    }
}

void LedController::setWifi(bool connected)
{
    digitalWrite(Pins::Led::WiFi, connected);
}

void LedController::setMqtt(bool connected)
{
    digitalWrite(Pins::Led::MQTT, connected);
}

void LedController::setError(bool error)
{
    digitalWrite(Pins::Led::Error, error);
}

void LedController::flashSend()
{
    digitalWrite(Pins::Led::Send, HIGH);
    sendLedActive = true;
    sendLedStartedAt = millis();
}
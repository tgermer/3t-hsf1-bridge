#include "RemoteController.h"
#include "Pins.h"

#include "Config.h"

void RemoteController::begin()
{
    pinMode(Pins::Remote::Open, OUTPUT);
    pinMode(Pins::Remote::Stop, OUTPUT);
    pinMode(Pins::Remote::Close, OUTPUT);

    digitalWrite(Pins::Remote::Open, LOW);
    digitalWrite(Pins::Remote::Stop, LOW);
    digitalWrite(Pins::Remote::Close, LOW);
}

void RemoteController::update()
{
    if (!buttonActive)
    {
        return;
    }

    if (millis() - buttonStartedAt >= buttonDurationMs)
    {
        digitalWrite(activePin, LOW);
        buttonActive = false;
    }
}

void RemoteController::pressOpen()
{
    pressButton(Pins::Remote::Open, Config::Remote::ButtonPressMs);
}

void RemoteController::pressStop()
{
    pressButton(Pins::Remote::Stop, Config::Remote::ButtonPressMs);
}

void RemoteController::pressClose()
{
    pressButton(Pins::Remote::Close, Config::Remote::ButtonPressMs);
}

void RemoteController::pressFavoritePosition()
{
    pressButton(Pins::Remote::Stop, Config::Remote::LongButtonPressMs);
}

void RemoteController::pressButton(uint8_t pin, uint32_t durationMs)
{
    if (buttonActive)
    {
        digitalWrite(activePin, LOW);
    }

    activePin = pin;
    buttonDurationMs = durationMs;
    buttonStartedAt = millis();
    buttonActive = true;

    digitalWrite(activePin, HIGH);
}
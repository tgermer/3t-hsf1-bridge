#include "RemoteController.h"
#include "Pins.h"

#include "Config.h"

void RemoteController::begin()
{
    pinMode(PIN_REMOTE_OPEN, OUTPUT);
    pinMode(PIN_REMOTE_STOP, OUTPUT);
    pinMode(PIN_REMOTE_CLOSE, OUTPUT);

    digitalWrite(PIN_REMOTE_OPEN, LOW);
    digitalWrite(PIN_REMOTE_STOP, LOW);
    digitalWrite(PIN_REMOTE_CLOSE, LOW);
}

void RemoteController::pressOpen()
{
    pressButton(PIN_REMOTE_OPEN, Config::Remote::ButtonPressMs);
}

void RemoteController::pressStop()
{
    pressButton(PIN_REMOTE_STOP, Config::Remote::ButtonPressMs);
}

void RemoteController::pressClose()
{
    pressButton(PIN_REMOTE_CLOSE, Config::Remote::ButtonPressMs);
}

void RemoteController::pressFavoritePosition()
{
    pressButton(PIN_REMOTE_STOP, Config::Remote::LongButtonPressMs);
}

void RemoteController::pressButton(uint8_t pin, uint32_t durationMs)
{
    digitalWrite(pin, HIGH);
    delay(durationMs);
    digitalWrite(pin, LOW);
}
#include "RemoteController.h"
#include "Pins.h"
#include "Constants.h"

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
    pressButton(PIN_REMOTE_OPEN, BUTTON_PRESS_MS);
}

void RemoteController::pressStop()
{
    pressButton(PIN_REMOTE_STOP, BUTTON_PRESS_MS);
}

void RemoteController::pressClose()
{
    pressButton(PIN_REMOTE_CLOSE, BUTTON_PRESS_MS);
}

void RemoteController::pressFavoritePosition()
{
    pressButton(PIN_REMOTE_STOP, LONG_BUTTON_PRESS_MS);
}

void RemoteController::pressButton(uint8_t pin, uint32_t durationMs)
{
    digitalWrite(pin, HIGH);
    delay(durationMs);
    digitalWrite(pin, LOW);
}
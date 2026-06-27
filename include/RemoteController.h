#pragma once

#include <Arduino.h>

class RemoteController
{
public:
    void begin();
    void update();

    void pressOpen();
    void pressStop();
    void pressClose();
    void pressFavoritePosition();

private:
    bool buttonActive = false;
    uint8_t activePin = 0;
    unsigned long buttonStartedAt = 0;
    uint32_t buttonDurationMs = 0;

    void pressButton(uint8_t pin, uint32_t durationMs);
};
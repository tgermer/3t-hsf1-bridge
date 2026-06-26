#pragma once

#include <Arduino.h>

class RemoteController
{
public:
    void begin();

    void pressOpen();
    void pressStop();
    void pressClose();
    void pressFavoritePosition();

private:
    void pressButton(uint8_t pin, uint32_t durationMs);
};
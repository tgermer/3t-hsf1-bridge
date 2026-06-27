#pragma once

#include <Arduino.h>

class LedController
{
public:
    void begin();
    void update();

    void setWifi(bool connected);
    void setMqtt(bool connected);
    void setError(bool error);

    void flashSend();

private:
    bool sendLedActive = false;
    unsigned long sendLedStartedAt = 0;

    static constexpr unsigned long SendFlashDurationMs = 100;
};
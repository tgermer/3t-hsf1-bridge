#pragma once

#include <Arduino.h>

namespace Pins
{
    namespace Remote
    {
        constexpr uint8_t Open = 16;
        constexpr uint8_t Stop = 17;
        constexpr uint8_t Close = 18;
    }

    namespace Led
    {
        constexpr uint8_t WiFi = 25;
        constexpr uint8_t MQTT = 26;
        constexpr uint8_t Send = 27;
        constexpr uint8_t Error = 14;
    }
}
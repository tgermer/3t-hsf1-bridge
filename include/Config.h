#pragma once

#include <Arduino.h>

namespace Config
{
    namespace WiFi
    {
        constexpr auto SSID = "GeschlosseneGesellschaft";
        constexpr auto Password = "Petrarcastrasse32";
        constexpr uint32_t ReconnectIntervalMs = 10000;
    }

    namespace MQTT
    {
        constexpr auto Host = "10.20.10.11";
        constexpr uint16_t Port = 1883;
        constexpr auto Username = "3t-hsf1-bridge";
        constexpr auto Password = "3t-hsf1-bridge-2026";
    }

    namespace Device
    {
        constexpr auto Name = "3T HSF1 Bridge";
        constexpr auto Version = "1.1.0";
    }

    namespace Remote
    {
        constexpr uint32_t ButtonPressMs = 300;
        constexpr uint32_t LongButtonPressMs = 3000;
    }

    namespace Awning
    {
        constexpr uint32_t OpenTimeMs = 26300;
        constexpr uint32_t CloseTimeMs = 27000;
        constexpr int SavedPositionPercent = 70;
    }
}

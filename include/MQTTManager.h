#pragma once

#include <Arduino.h>
#include <ArduinoHA.h>
#include <WiFi.h>

class MQTTManager
{
public:
    MQTTManager();

    void begin();
    void connect();
    void update();

    bool isConnected() const;

    HAMqtt &getMqtt();
    HADevice &getDevice();

private:
    WiFiClient wifiClient;

    byte mac[6] = {0xA5, 0x5C, 0xDB, 0xDF, 0x7B, 0x37};

    HADevice device;
    HAMqtt mqtt;

    bool lastConnectionState = false;
};
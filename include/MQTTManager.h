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
    static byte *getHardwareMacAddress();
    WiFiClient wifiClient;

    HADevice device;
    HAMqtt mqtt;

    bool lastConnectionState = false;
};
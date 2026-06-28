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
    bool isStarted() const;
    uint32_t getReconnectCount() const;

    HAMqtt &getMqtt();
    HADevice &getDevice();

private:
    WiFiClient wifiClient;

    HADevice device;
    HAMqtt mqtt;

    bool started = false;
    bool lastConnectionState = false;
    bool hasConnected = false;
    uint32_t reconnectCount = 0;

    unsigned long lastConnectAttemptMs = 0;

    static constexpr unsigned long ReconnectIntervalMs = 10000;
};

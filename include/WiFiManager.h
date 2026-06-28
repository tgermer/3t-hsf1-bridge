#pragma once

#include <Arduino.h>

enum class WiFiConnectionState
{
    Disconnected,
    Connecting,
    Connected
};

class WiFiManager
{
public:
    void begin();
    void update();

    bool isConnected() const;
    String getIpAddress() const;
    String getSSID() const;
    uint32_t getReconnectCount() const;
    WiFiConnectionState getState() const;

private:
    WiFiConnectionState state = WiFiConnectionState::Disconnected;
    bool hasConnected = false;
    uint32_t reconnectCount = 0;
    unsigned long lastReconnectAttemptMs = 0;

    void startConnection();
    void handleConnected();
    void handleDisconnected();
};

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
    WiFiConnectionState getState() const;

private:
    WiFiConnectionState state = WiFiConnectionState::Disconnected;
    unsigned long lastReconnectAttemptMs = 0;

    void startConnection();
    void handleConnected();
    void handleDisconnected();
};
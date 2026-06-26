#include "WiFiManager.h"

#include <WiFi.h>

#include "Config.h"
#include "Logger.h"

constexpr unsigned long WIFI_RECONNECT_INTERVAL_MS = 10000;

void WiFiManager::begin()
{
    WiFi.mode(WIFI_STA);
    startConnection();
}

void WiFiManager::update()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        handleConnected();
    }
    else
    {
        handleDisconnected();
    }
}

bool WiFiManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIpAddress() const
{
    if (!isConnected())
    {
        return "";
    }

    return WiFi.localIP().toString();
}

String WiFiManager::getSSID() const
{
    return WIFI_SSID;
}

WiFiConnectionState WiFiManager::getState() const
{
    return state;
}

void WiFiManager::startConnection()
{
    Logger::info("Connecting to WiFi: " + String(WIFI_SSID));

    state = WiFiConnectionState::Connecting;
    lastReconnectAttemptMs = millis();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiManager::handleConnected()
{
    if (state != WiFiConnectionState::Connected)
    {
        state = WiFiConnectionState::Connected;
        Logger::info("WiFi connected, IP: " + getIpAddress());
    }
}

void WiFiManager::handleDisconnected()
{
    if (state == WiFiConnectionState::Connected)
    {
        Logger::warning("WiFi disconnected");
    }

    state = WiFiConnectionState::Disconnected;

    unsigned long now = millis();

    if (now - lastReconnectAttemptMs >= WIFI_RECONNECT_INTERVAL_MS)
    {
        Logger::info("Retrying WiFi connection");
        startConnection();
    }
}
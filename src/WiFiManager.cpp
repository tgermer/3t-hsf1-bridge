#include "WiFiManager.h"

#include <WiFi.h>

#include "Config.h"
#include "Logger.h"

void WiFiManager::begin()
{
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
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
    return state == WiFiConnectionState::Connected;
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
    return Config::WiFi::SSID;
}

WiFiConnectionState WiFiManager::getState() const
{
    return state;
}

void WiFiManager::startConnection()
{
    Logger::info("Connecting to WiFi: " + String(Config::WiFi::SSID));

    state = WiFiConnectionState::Connecting;
    lastReconnectAttemptMs = millis();

    WiFi.begin(Config::WiFi::SSID, Config::WiFi::Password);
}

void WiFiManager::handleConnected()
{
    if (WiFi.localIP().toString() == "0.0.0.0")
    {
        return;
    }

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

    if (now - lastReconnectAttemptMs >= Config::WiFi::ReconnectIntervalMs)
    {
        Logger::info("Retrying WiFi connection");
        startConnection();
    }
}
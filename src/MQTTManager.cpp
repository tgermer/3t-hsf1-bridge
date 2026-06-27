#include "MQTTManager.h"

#include "Config.h"
#include "Logger.h"

MQTTManager::MQTTManager()
    : mqtt(wifiClient, device),
      started(false),
      lastConnectionState(false)
{
}

void MQTTManager::begin()
{
    Logger::info("Initializing MQTT manager");

    byte macAddress[6];
    WiFi.macAddress(macAddress);
    device.setUniqueId(macAddress, sizeof(macAddress));

    Logger::info(
        "Device MAC address: " +
        String(macAddress[0], HEX) + ":" +
        String(macAddress[1], HEX) + ":" +
        String(macAddress[2], HEX) + ":" +
        String(macAddress[3], HEX) + ":" +
        String(macAddress[4], HEX) + ":" +
        String(macAddress[5], HEX));

    device.setName(Config::Device::Name);
    device.setSoftwareVersion(Config::Device::Version);
}

void MQTTManager::connect()
{
    // Avoid rapid reconnect loops.
    // Mosquitto may close stale connections after keepalive timeouts.
    // Waiting between attempts prevents repeated session takeovers.
    if (started && isConnected())
    {
        return;
    }

    unsigned long now = millis();

    if (lastConnectAttemptMs > 0 &&
        now - lastConnectAttemptMs < ReconnectIntervalMs)
    {
        return;
    }

    lastConnectAttemptMs = now;

    Logger::info(
        "Connecting MQTT broker " +
        String(Config::MQTT::Host) +
        ":" +
        String(Config::MQTT::Port));

    // Initializes the MQTT connection and starts automatic reconnect handling.
    mqtt.setBufferSize(512);
    mqtt.setKeepAlive(60);

    mqtt.begin(
        Config::MQTT::Host,
        Config::MQTT::Port,
        Config::MQTT::Username,
        Config::MQTT::Password);

    started = true;

    Logger::info("MQTT begin called");
}

void MQTTManager::update()
{
    if (!started)
    {
        return;
    }

    mqtt.loop();

    bool connected = isConnected();

    if (connected != lastConnectionState)
    {
        if (connected)
        {
            Logger::info("MQTT connected");
        }
        else
        {
            Logger::warning("MQTT disconnected");
            started = false; // Allow a fresh mqtt.begin(...) on the next reconnect attempt.
        }

        lastConnectionState = connected;
    }
}

bool MQTTManager::isConnected() const
{
    return mqtt.isConnected();
}

bool MQTTManager::isStarted() const
{
    return started;
}

HAMqtt &MQTTManager::getMqtt()
{
    return mqtt;
}

HADevice &MQTTManager::getDevice()
{
    return device;
}
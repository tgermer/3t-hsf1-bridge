#include "MQTTManager.h"

#include "Config.h"
#include "Logger.h"

MQTTManager::MQTTManager()
    : device(macAddress, sizeof(macAddress)),
      mqtt(wifiClient, device)
{
}

void MQTTManager::begin()
{
    Logger::info("Initializing MQTT manager");

    WiFi.macAddress(macAddress);
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
    Logger::info(
        "Connecting MQTT broker " +
        String(Config::MQTT::Host) +
        ":" +
        String(Config::MQTT::Port));

    // Initializes the MQTT connection and starts automatic reconnect handling.
    mqtt.begin(
        Config::MQTT::Host,
        Config::MQTT::Port,
        Config::MQTT::Username,
        Config::MQTT::Password);

    Logger::info("MQTT begin called");
}

void MQTTManager::update()
{
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
        }

        lastConnectionState = connected;
    }
}

bool MQTTManager::isConnected() const
{
    return mqtt.isConnected();
}

HAMqtt &MQTTManager::getMqtt()
{
    return mqtt;
}

HADevice &MQTTManager::getDevice()
{
    return device;
}
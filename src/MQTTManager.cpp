#include "MQTTManager.h"

#include "Config.h"
#include "Logger.h"

MQTTManager::MQTTManager()
    : device(mac, sizeof(mac)),
      mqtt(wifiClient, device)
{
}

void MQTTManager::begin()
{
    Logger::info("Initializing MQTT manager");

    device.setName(Config::Device::Name);
    device.setSoftwareVersion(Config::Device::Version);
}

void MQTTManager::connect()
{
    Logger::info("Connecting MQTT");

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
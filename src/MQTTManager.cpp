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

    device.setName("3T HSF1 Bridge");
    device.setSoftwareVersion("0.1.0");

    mqtt.begin(
        MQTT_HOST,
        MQTT_PORT,
        MQTT_USERNAME,
        MQTT_PASSWORD);

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
#include "MQTTManager.h"

#include "Config.h"
#include "Logger.h"

byte *MQTTManager::getHardwareMacAddress()
{
    static byte macAddress[6] = {0};

    uint64_t chipId = ESP.getEfuseMac();

    macAddress[0] = static_cast<byte>(chipId >> 0);
    macAddress[1] = static_cast<byte>(chipId >> 8);
    macAddress[2] = static_cast<byte>(chipId >> 16);
    macAddress[3] = static_cast<byte>(chipId >> 24);
    macAddress[4] = static_cast<byte>(chipId >> 32);
    macAddress[5] = static_cast<byte>(chipId >> 40);

    return macAddress;
}

MQTTManager::MQTTManager()
    : device(getHardwareMacAddress(), 6),
      mqtt(wifiClient, device),
      started(false),
      lastConnectionState(false)
{
}

void MQTTManager::begin()
{
    Logger::info("Initializing MQTT manager");

    byte *macAddress = getHardwareMacAddress();
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
    if (started)
    {
        return;
    }

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

    unsigned long now = millis();
    if (now - lastLoopLogMs >= 5000)
    {
        lastLoopLogMs = now;
        Logger::info("MQTT loop running, connected=" + String(isConnected() ? "true" : "false"));
    }

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
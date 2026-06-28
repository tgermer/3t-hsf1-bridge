#include "NativeCoverMqtt.h"

#include "Config.h"
#include "Logger.h"

NativeCoverMqtt *NativeCoverMqtt::instance = nullptr;

NativeCoverMqtt::NativeCoverMqtt(
    MQTTManager &mqttManager,
    NativePositionCover &cover)
    : mqttManager(mqttManager),
      cover(cover)
{
}

void NativeCoverMqtt::begin(SetPositionCommandCallback callback)
{
    instance = this;
    setPositionCommandCallback = callback;
    configureNativePositionMqtt();
    coverSetupComplete = true;
}

void NativeCoverMqtt::update()
{
    if (mqttManager.isConnected() &&
        coverMqttSetupPending &&
        millis() - lastCoverMqttSetupAttemptMs >= CoverMqttSetupRetryMs)
    {
        setupCoverMqttConnection();
    }
}

void NativeCoverMqtt::synchronize()
{
    removeLegacyTargetPositionDiscovery();
}

void NativeCoverMqtt::configureNativePositionMqtt()
{
    HAMqtt &mqtt = mqttManager.getMqtt();
    const char *deviceId = mqttManager.getDevice().getUniqueId();

    coverCommandTopic = String(mqtt.getDataPrefix()) +
                        "/" + deviceId +
                        "/markise/cmd_t";
    coverPositionCommandTopic = String(mqtt.getDataPrefix()) +
                                "/" + deviceId +
                                "/markise/set_position";
    coverDiscoveryTopic = String(mqtt.getDiscoveryPrefix()) +
                          "/cover/" + deviceId +
                          "/markise/config";

    mqtt.onMessage(onMqttMessage);
    cover.setMqttConnectedCallback(onCoverMqttConnected);
}

void NativeCoverMqtt::setupCoverMqttConnection()
{
    lastCoverMqttSetupAttemptMs = millis();

    if (!coverSetupComplete)
    {
        coverMqttSetupPending = true;
        Logger::warning("Cover MQTT setup deferred until cover configuration is complete");
        return;
    }

    HAMqtt &mqtt = mqttManager.getMqtt();
    bool commandSubscribed = mqtt.subscribe(coverCommandTopic.c_str());
    bool positionCommandSubscribed = mqtt.subscribe(coverPositionCommandTopic.c_str());
    bool discoveryPublished =
        commandSubscribed &&
        positionCommandSubscribed &&
        publishCoverDiscovery();

    coverMqttSetupPending =
        !discoveryPublished ||
        !commandSubscribed ||
        !positionCommandSubscribed;

    if (coverMqttSetupPending)
    {
        Logger::warning(
            "Cover MQTT setup incomplete; native position discovery retry scheduled");
        return;
    }

    Logger::info(
        "Cover MQTT discovery published: native position support enabled");
}

bool NativeCoverMqtt::publishCoverDiscovery()
{
    HAMqtt &mqtt = mqttManager.getMqtt();
    const char *deviceId = mqttManager.getDevice().getUniqueId();
    String dataTopic = String(mqtt.getDataPrefix()) +
                       "/" + deviceId +
                       "/markise/";
    String availabilityTopic = String(mqtt.getDataPrefix()) +
                               "/" + deviceId +
                               "/avty_t";

    String payload;
    payload.reserve(700);
    payload += "{\"name\":\"Markise\",";
    payload += "\"unique_id\":\"markise\",";
    payload += "\"device_class\":\"awning\",";
    payload += "\"icon\":\"mdi:awning-outline\",";
    payload += "\"state_topic\":\"";
    payload += dataTopic;
    payload += "stat_t\",";
    payload += "\"command_topic\":\"";
    payload += dataTopic;
    payload += "cmd_t\",";
    payload += "\"position_topic\":\"";
    payload += dataTopic;
    payload += "pos_t\",";
    payload += "\"set_position_topic\":\"";
    payload += coverPositionCommandTopic;
    payload += "\",";
    payload += "\"position_open\":100,";
    payload += "\"position_closed\":0,";
    payload += "\"availability_topic\":\"";
    payload += availabilityTopic;
    payload += "\",";
    payload += "\"device\":{\"identifiers\":[\"";
    payload += deviceId;
    payload += "\"],\"name\":\"";
    payload += Config::Device::Name;
    payload += "\",\"sw_version\":\"";
    payload += Config::Device::Version;
    payload += "\"}}";

    return mqtt.publish(coverDiscoveryTopic.c_str(), payload.c_str(), true);
}

void NativeCoverMqtt::removeLegacyTargetPositionDiscovery()
{
    HAMqtt &mqtt = mqttManager.getMqtt();
    const char *deviceId = mqttManager.getDevice().getUniqueId();
    String discoveryTopic = String(mqtt.getDiscoveryPrefix()) +
                            "/number/" + deviceId +
                            "/markise_target_position/config";

    if (!mqtt.publish(discoveryTopic.c_str(), "", true))
    {
        Logger::warning("Failed to remove legacy target-position discovery");
    }
}

void NativeCoverMqtt::onCoverMqttConnected()
{
    if (instance != nullptr)
    {
        instance->setupCoverMqttConnection();
    }
}

void NativeCoverMqtt::onMqttMessage(
    const char *topic,
    const uint8_t *payload,
    uint16_t length)
{
    if (instance != nullptr)
    {
        instance->handleMqttMessage(topic, payload, length);
    }
}

void NativeCoverMqtt::handleMqttMessage(
    const char *topic,
    const uint8_t *payload,
    uint16_t length)
{
    if (strcmp(topic, coverPositionCommandTopic.c_str()) != 0)
    {
        return;
    }

    if (setPositionCommandCallback != nullptr)
    {
        setPositionCommandCallback(payload, length);
    }
}

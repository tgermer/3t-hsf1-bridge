#pragma once

#include <ArduinoHA.h>

#include "MQTTManager.h"

class NativePositionCover : public HACover
{
public:
    using MqttConnectedCallback = void (*)();

    NativePositionCover(const char *uniqueId, Features features)
        : HACover(uniqueId, features)
    {
    }

    void setMqttConnectedCallback(MqttConnectedCallback callback)
    {
        mqttConnectedCallback = callback;
    }

protected:
    void onMqttConnected() override
    {
        // HACover discovery lacks set_position_topic in this ArduinoHA version.
        // NativeCoverMqtt owns the complete cover discovery and subscriptions instead.
        if (mqttConnectedCallback != nullptr)
        {
            mqttConnectedCallback();
        }
    }

private:
    MqttConnectedCallback mqttConnectedCallback = nullptr;
};

class NativeCoverMqtt
{
public:
    using SetPositionCommandCallback = void (*)(
        const uint8_t *payload,
        uint16_t length);

    NativeCoverMqtt(
        MQTTManager &mqttManager,
        NativePositionCover &cover);

    void begin(SetPositionCommandCallback callback);
    void update();
    void synchronize();

private:
    MQTTManager &mqttManager;
    NativePositionCover &cover;
    SetPositionCommandCallback setPositionCommandCallback = nullptr;

    String coverCommandTopic;
    String coverPositionCommandTopic;
    String coverDiscoveryTopic;

    bool coverSetupComplete = false;
    bool coverMqttSetupPending = false;
    unsigned long lastCoverMqttSetupAttemptMs = 0;

    static constexpr unsigned long CoverMqttSetupRetryMs = 5000;

    static NativeCoverMqtt *instance;
    static void onCoverMqttConnected();
    static void onMqttMessage(
        const char *topic,
        const uint8_t *payload,
        uint16_t length);

    void configureNativePositionMqtt();
    void setupCoverMqttConnection();
    bool publishCoverDiscovery();
    void removeLegacyTargetPositionDiscovery();
    void handleMqttMessage(
        const char *topic,
        const uint8_t *payload,
        uint16_t length);
};

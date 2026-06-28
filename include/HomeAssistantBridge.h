#pragma once

#include <ArduinoHA.h>
#include <Preferences.h>

#include "LedController.h"
#include "MQTTManager.h"
#include "PositionTracker.h"
#include "RemoteController.h"
#include "WiFiManager.h"

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
        // The bridge owns the complete cover discovery and subscriptions instead.
        if (mqttConnectedCallback != nullptr)
        {
            mqttConnectedCallback();
        }
    }

private:
    MqttConnectedCallback mqttConnectedCallback = nullptr;
};

class HomeAssistantBridge
{
public:
    HomeAssistantBridge(
        WiFiManager &wifiManager,
        MQTTManager &mqttManager,
        RemoteController &remote,
        PositionTracker &position,
        LedController &leds);

    void begin();
    void update();

private:
    WiFiManager &wifiManager;
    MQTTManager &mqttManager;
    RemoteController &remote;
    PositionTracker &position;
    LedController &leds;

    NativePositionCover awningCover;
    HAButton savedPositionButton;
    HANumber savedPositionAssumedPercentNumber;
    HASensor wifiSsidSensor;
    HASensor ipAddressSensor;
    HASensor macAddressSensor;
    HASensorNumber wifiRssiSensor;
    HASensorNumber freeHeapSensor;
    HASensor resetReasonSensor;
    HASensorNumber wifiReconnectCounterSensor;
    HASensorNumber mqttReconnectCounterSensor;
    HAButton restartButton;
    String restartCommandTopic;
    String coverCommandTopic;
    String coverPositionCommandTopic;
    String coverDiscoveryTopic;
    Preferences preferences;

    int lastPublishedPosition = -1;
    HACover::CoverState lastPublishedState = HACover::StateUnknown;
    unsigned long lastPositionPublishMs = 0;
    bool lastMqttConnected = false;
    bool targetPositionActive = false;
    bool targetPositionRequiresPhysicalStop = true;
    bool pendingMovementStartsTarget = false;
    int targetPosition = -1;
    int savedPositionAssumedPercent = 0;
    bool preferencesReady = false;
    bool coverSetupComplete = false;
    bool coverMqttSetupPending = false;
    bool diagnosticsPublishPending = true;
    bool restartSubscriptionPending = true;
    unsigned long lastCoverMqttSetupAttemptMs = 0;
    unsigned long lastDiagnosticsPublishAttemptMs = 0;
    unsigned long lastDiagnosticsPublishMs = 0;
    unsigned long lastRestartSubscriptionAttemptMs = 0;

    static constexpr unsigned long PositionPublishIntervalMs = 1000;
    static constexpr unsigned long CoverMqttSetupRetryMs = 5000;
    static constexpr unsigned long DiagnosticsPublishRetryMs = 5000;
    static constexpr unsigned long DiagnosticsPublishIntervalMs = 60000;
    static constexpr unsigned long RestartSubscriptionRetryMs = 5000;
    static constexpr int TargetPositionTolerance = 1;

    static HomeAssistantBridge *instance;
    static void onCoverCommand(HACover::CoverCommand cmd, HACover *sender);
    static void onSavedPositionCommand(HAButton *sender);
    static void onSavedPositionAssumedPercentCommand(HANumeric number, HANumber *sender);
    static void onRestartCommand(HAButton *sender);
    static void onDiagnosticsMqttConnected();
    static void onRemoteCommandStarted(RemoteController::Command command);
    static void onCoverMqttConnected();
    static void onMqttMessage(const char *topic, const uint8_t *payload, uint16_t length);

    void publishPositionIfNeeded();
    bool publishPosition(bool force = false);
    void publishFinalPosition();
    void publishCoverState();
    void publishCoverState(HACover::CoverState state, bool force = false);
    bool publishDiagnostics(bool force = false);
    bool publishStaticDiagnostics();
    void subscribeRestartCommand();
    HACover::CoverState getCoverState() const;
    void synchronizeMqttState();
    void configureNativePositionMqtt();
    void setupCoverMqttConnection();
    bool publishCoverDiscovery();
    void removeLegacyTargetPositionDiscovery();
    void removeLegacyDiagnosticsDiscovery();
    void handleMqttMessage(const char *topic, const uint8_t *payload, uint16_t length);
    void updateTargetPositionMovement();

    void handleCoverCommand(HACover::CoverCommand cmd);
    void handleSavedPositionCommand();
    void handleSavedPositionAssumedPercentCommand(HANumeric number);
    void handleRemoteCommandStarted(RemoteController::Command command);
    void loadSavedPositionAssumedPercent();
    void moveToTargetPosition(int requestedPosition);
};

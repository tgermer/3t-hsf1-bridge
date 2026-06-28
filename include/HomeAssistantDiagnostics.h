#pragma once

#include <ArduinoHA.h>

#include "MQTTManager.h"
#include "WiFiManager.h"

class HomeAssistantDiagnostics
{
public:
    HomeAssistantDiagnostics(
        WiFiManager &wifiManager,
        MQTTManager &mqttManager);

    void begin();
    void update();
    void synchronize();

private:
    WiFiManager &wifiManager;
    MQTTManager &mqttManager;

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

    bool diagnosticsPublishPending = true;
    bool restartSubscriptionPending = true;
    unsigned long lastDiagnosticsPublishAttemptMs = 0;
    unsigned long lastDiagnosticsPublishMs = 0;
    unsigned long lastRestartSubscriptionAttemptMs = 0;

    static constexpr unsigned long DiagnosticsPublishRetryMs = 5000;
    static constexpr unsigned long DiagnosticsPublishIntervalMs = 60000;
    static constexpr unsigned long RestartSubscriptionRetryMs = 5000;

    static HomeAssistantDiagnostics *instance;
    static void onRestartCommand(HAButton *sender);
    static void onMqttConnected();

    bool publish(bool force = false);
    bool publishStatic();
    void subscribeRestartCommand();
    void removeLegacyDiscovery();
};

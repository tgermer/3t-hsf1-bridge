#include "HomeAssistantDiagnostics.h"

#include "Logger.h"

#include <WiFi.h>
#include <esp_system.h>

namespace
{
    const char *resetReasonToString(esp_reset_reason_t reason)
    {
        switch (reason)
        {
        case ESP_RST_POWERON:
            return "power_on";
        case ESP_RST_EXT:
            return "external";
        case ESP_RST_SW:
            return "software";
        case ESP_RST_PANIC:
            return "panic";
        case ESP_RST_INT_WDT:
            return "interrupt_watchdog";
        case ESP_RST_TASK_WDT:
            return "task_watchdog";
        case ESP_RST_WDT:
            return "watchdog";
        case ESP_RST_DEEPSLEEP:
            return "deep_sleep";
        case ESP_RST_BROWNOUT:
            return "brownout";
        case ESP_RST_SDIO:
            return "sdio";
        default:
            return "unknown";
        }
    }
}

HomeAssistantDiagnostics *HomeAssistantDiagnostics::instance = nullptr;

HomeAssistantDiagnostics::HomeAssistantDiagnostics(
    WiFiManager &wifiManager,
    MQTTManager &mqttManager)
    : wifiManager(wifiManager),
      mqttManager(mqttManager),
      wifiSsidSensor("wifi_ssid"),
      ipAddressSensor("ip_address"),
      macAddressSensor("mac_address"),
      wifiRssiSensor("wifi_rssi"),
      freeHeapSensor("free_heap"),
      resetReasonSensor("reset_reason"),
      wifiReconnectCounterSensor("wifi_reconnect_count"),
      mqttReconnectCounterSensor("mqtt_reconnect_count"),
      restartButton("restart")
{
}

void HomeAssistantDiagnostics::begin()
{
    instance = this;

    wifiSsidSensor.setName("WLAN SSID");
    wifiSsidSensor.setIcon("mdi:wifi-settings");

    ipAddressSensor.setName("IP-Adresse");
    ipAddressSensor.setIcon("mdi:ip-network");

    macAddressSensor.setName("MAC-Adresse");
    macAddressSensor.setIcon("mdi:network-outline");

    wifiRssiSensor.setName("WLAN-Signalstärke");
    wifiRssiSensor.setDeviceClass("signal_strength");
    wifiRssiSensor.setStateClass("measurement");
    wifiRssiSensor.setUnitOfMeasurement("dBm");
    wifiRssiSensor.setIcon("mdi:wifi");
    wifiRssiSensor.setCurrentValue(static_cast<int32_t>(WiFi.RSSI()));

    freeHeapSensor.setName("Freier Heap");
    freeHeapSensor.setDeviceClass("data_size");
    freeHeapSensor.setStateClass("measurement");
    freeHeapSensor.setUnitOfMeasurement("kB");
    freeHeapSensor.setIcon("mdi:memory");
    freeHeapSensor.setCurrentValue(static_cast<uint32_t>(ESP.getFreeHeap() / 1000U));

    resetReasonSensor.setName("Reset-Grund");
    resetReasonSensor.setIcon("mdi:restart-alert");

    wifiReconnectCounterSensor.setName("WLAN-Neuverbindungen");
    wifiReconnectCounterSensor.setStateClass("total_increasing");
    wifiReconnectCounterSensor.setIcon("mdi:wifi-sync");
    wifiReconnectCounterSensor.setCurrentValue(wifiManager.getReconnectCount());

    mqttReconnectCounterSensor.setName("MQTT-Neuverbindungen");
    mqttReconnectCounterSensor.setStateClass("total_increasing");
    mqttReconnectCounterSensor.setIcon("mdi:sync");
    mqttReconnectCounterSensor.setCurrentValue(mqttManager.getReconnectCount());

    restartButton.setName("Neustart");
    restartButton.setDeviceClass("restart");
    restartButton.setIcon("mdi:restart");
    restartButton.onCommand(onRestartCommand);

    HAMqtt &mqtt = mqttManager.getMqtt();
    const char *deviceId = mqttManager.getDevice().getUniqueId();
    restartCommandTopic = String(mqtt.getDataPrefix()) +
                          "/" + deviceId +
                          "/restart/cmd_t";
    mqtt.onConnected(onMqttConnected);
}

void HomeAssistantDiagnostics::update()
{
    if (!mqttManager.isConnected())
    {
        return;
    }

    if (restartSubscriptionPending &&
        (lastRestartSubscriptionAttemptMs == 0 ||
         millis() - lastRestartSubscriptionAttemptMs >= RestartSubscriptionRetryMs))
    {
        subscribeRestartCommand();
    }

    unsigned long now = millis();
    bool diagnosticsRetryDue =
        diagnosticsPublishPending &&
        now - lastDiagnosticsPublishAttemptMs >= DiagnosticsPublishRetryMs;
    bool diagnosticsUpdateDue =
        !diagnosticsPublishPending &&
        now - lastDiagnosticsPublishMs >= DiagnosticsPublishIntervalMs;

    if (diagnosticsRetryDue || diagnosticsUpdateDue)
    {
        publish(diagnosticsPublishPending);
    }
}

void HomeAssistantDiagnostics::synchronize()
{
    removeLegacyDiscovery();
    publish(true);
}

bool HomeAssistantDiagnostics::publish(bool force)
{
    lastDiagnosticsPublishAttemptMs = millis();

    int32_t wifiRssi = WiFi.RSSI();
    uint32_t freeHeapKb = ESP.getFreeHeap() / 1000U;
    uint32_t wifiReconnectCount = wifiManager.getReconnectCount();
    uint32_t mqttReconnectCount = mqttManager.getReconnectCount();

    bool staticPublished = !force || publishStatic();
    bool wifiRssiPublished = wifiRssiSensor.setValue(wifiRssi, force);
    bool freeHeapPublished = freeHeapSensor.setValue(freeHeapKb, force);
    bool wifiReconnectCountPublished =
        wifiReconnectCounterSensor.setValue(wifiReconnectCount, force);
    bool mqttReconnectCountPublished =
        mqttReconnectCounterSensor.setValue(mqttReconnectCount, force);

    if (staticPublished &&
        wifiRssiPublished &&
        freeHeapPublished &&
        wifiReconnectCountPublished &&
        mqttReconnectCountPublished)
    {
        diagnosticsPublishPending = false;
        lastDiagnosticsPublishMs = millis();

        if (force)
        {
            Logger::info("Home Assistant diagnostics state published");
        }

        return true;
    }

    diagnosticsPublishPending = true;
    Logger::warning("Failed to publish Home Assistant diagnostics; retry scheduled");
    return false;
}

bool HomeAssistantDiagnostics::publishStatic()
{
    String ssid = WiFi.SSID();
    String ipAddress = WiFi.localIP().toString();
    String macAddress = WiFi.macAddress();

    bool ssidPublished = wifiSsidSensor.setValue(ssid.c_str());
    bool ipAddressPublished = ipAddressSensor.setValue(ipAddress.c_str());
    bool macAddressPublished = macAddressSensor.setValue(macAddress.c_str());
    bool resetReasonPublished =
        resetReasonSensor.setValue(resetReasonToString(esp_reset_reason()));

    return ssidPublished &&
           ipAddressPublished &&
           macAddressPublished &&
           resetReasonPublished;
}

void HomeAssistantDiagnostics::subscribeRestartCommand()
{
    lastRestartSubscriptionAttemptMs = millis();
    restartSubscriptionPending =
        !mqttManager.getMqtt().subscribe(restartCommandTopic.c_str());

    if (restartSubscriptionPending)
    {
        Logger::warning("Failed to subscribe to restart commands; retry scheduled");
    }
    else
    {
        Logger::info("Home Assistant restart command subscription active");
    }
}

void HomeAssistantDiagnostics::removeLegacyDiscovery()
{
    HAMqtt &mqtt = mqttManager.getMqtt();
    const char *deviceId = mqttManager.getDevice().getUniqueId();
    String discoveryPrefix = String(mqtt.getDiscoveryPrefix()) +
                             "/sensor/" + deviceId + "/";

    bool firmwareRemoved = mqtt.publish(
        (discoveryPrefix + "firmware_version/config").c_str(),
        "",
        true);
    bool uptimeRemoved = mqtt.publish(
        (discoveryPrefix + "uptime/config").c_str(),
        "",
        true);
    bool bootTimestampRemoved = mqtt.publish(
        (discoveryPrefix + "boot_timestamp/config").c_str(),
        "",
        true);

    if (!firmwareRemoved || !uptimeRemoved || !bootTimestampRemoved)
    {
        Logger::warning("Failed to remove legacy diagnostics discovery");
    }
}

void HomeAssistantDiagnostics::onRestartCommand(HAButton *sender)
{
    (void)sender;
    Logger::info("Home Assistant command: Restart");
    Serial.flush();
    ESP.restart();
}

void HomeAssistantDiagnostics::onMqttConnected()
{
    if (instance != nullptr)
    {
        instance->diagnosticsPublishPending = true;
        instance->lastDiagnosticsPublishAttemptMs =
            millis() - DiagnosticsPublishRetryMs;
        instance->restartSubscriptionPending = true;
        instance->lastRestartSubscriptionAttemptMs = 0;
    }
}

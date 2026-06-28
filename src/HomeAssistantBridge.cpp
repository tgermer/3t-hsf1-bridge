#include "HomeAssistantBridge.h"

#include "Config.h"
#include "Logger.h"

#include <WiFi.h>
#include <esp_timer.h>

namespace
{
    constexpr auto PreferencesNamespace = "ha-bridge";
    constexpr auto SavedPositionPercentKey = "savedPercent";
}

HomeAssistantBridge *HomeAssistantBridge::instance = nullptr;

HomeAssistantBridge::HomeAssistantBridge(
    MQTTManager &mqttManager,
    RemoteController &remote,
    PositionTracker &position,
    LedController &leds)
    : mqttManager(mqttManager),
      remote(remote),
      position(position),
      leds(leds),
      awningCover("markise", HACover::PositionFeature),
      savedPositionButton("markise_saved_position"),
      savedPositionAssumedPercentNumber("markise_saved_position_assumed_percent"),
      firmwareVersionSensor("firmware_version"),
      uptimeSensor("uptime"),
      wifiRssiSensor("wifi_rssi"),
      restartButton("restart")
{
}

void HomeAssistantBridge::begin()
{
    Logger::info("Initializing Home Assistant bridge");

    instance = this;
    loadSavedPositionAssumedPercent();
    remote.onCommandStarted(onRemoteCommandStarted);

    awningCover.setName("Markise");
    awningCover.setDeviceClass("awning");
    awningCover.setIcon("mdi:awning-outline");
    awningCover.setOptimistic(false);
    awningCover.onCommand(onCoverCommand);

    int initialPosition = constrain(position.getPosition(), 0, 100);
    if (awningCover.setPosition(initialPosition))
    {
        lastPublishedPosition = initialPosition;
    }

    publishCoverState();

    savedPositionButton.setName("Gespeicherte Position");
    savedPositionButton.setIcon("mdi:star-outline");
    savedPositionButton.onCommand(onSavedPositionCommand);

    savedPositionAssumedPercentNumber.setName("Angenommene gespeicherte Position");
    savedPositionAssumedPercentNumber.setMin(0);
    savedPositionAssumedPercentNumber.setMax(100);
    savedPositionAssumedPercentNumber.setStep(1);
    savedPositionAssumedPercentNumber.setMode(HANumber::ModeSlider);
    savedPositionAssumedPercentNumber.onCommand(onSavedPositionAssumedPercentCommand);
    savedPositionAssumedPercentNumber.setState(savedPositionAssumedPercent);

    firmwareVersionSensor.setName("Firmware-Version");
    firmwareVersionSensor.setIcon("mdi:information-outline");

    uptimeSensor.setName("Laufzeit");
    uptimeSensor.setDeviceClass("duration");
    uptimeSensor.setStateClass("total_increasing");
    uptimeSensor.setUnitOfMeasurement("s");
    uptimeSensor.setIcon("mdi:timer-outline");
    uptimeSensor.setCurrentValue(static_cast<uint32_t>(
        esp_timer_get_time() / 1000000ULL));

    wifiRssiSensor.setName("WLAN-Signalstärke");
    wifiRssiSensor.setDeviceClass("signal_strength");
    wifiRssiSensor.setStateClass("measurement");
    wifiRssiSensor.setUnitOfMeasurement("dBm");
    wifiRssiSensor.setIcon("mdi:wifi");
    wifiRssiSensor.setCurrentValue(static_cast<int32_t>(WiFi.RSSI()));

    restartButton.setName("Neustart");
    restartButton.setDeviceClass("restart");
    restartButton.setIcon("mdi:restart");
    restartButton.onCommand(onRestartCommand);

    HAMqtt &mqtt = mqttManager.getMqtt();
    const char *deviceId = mqttManager.getDevice().getUniqueId();
    restartCommandTopic = String(mqtt.getDataPrefix()) +
                          "/" + deviceId +
                          "/restart/cmd_t";
    mqtt.onConnected(onDiagnosticsMqttConnected);
    configureNativePositionMqtt();
    coverSetupComplete = true;

    Logger::info(
        "Home Assistant cover setup complete: native position support enabled");
}

void HomeAssistantBridge::update()
{
    bool mqttConnected = mqttManager.isConnected();

    if (mqttConnected &&
        coverMqttSetupPending &&
        millis() - lastCoverMqttSetupAttemptMs >= CoverMqttSetupRetryMs)
    {
        setupCoverMqttConnection();
    }

    if (mqttConnected &&
        restartSubscriptionPending &&
        (lastRestartSubscriptionAttemptMs == 0 ||
         millis() - lastRestartSubscriptionAttemptMs >= RestartSubscriptionRetryMs))
    {
        subscribeRestartCommand();
    }

    if (mqttConnected && !lastMqttConnected)
    {
        synchronizeMqttState();
    }

    lastMqttConnected = mqttConnected;

    unsigned long now = millis();
    bool diagnosticsRetryDue =
        diagnosticsPublishPending &&
        now - lastDiagnosticsPublishAttemptMs >= DiagnosticsPublishRetryMs;
    bool diagnosticsUpdateDue =
        !diagnosticsPublishPending &&
        now - lastDiagnosticsPublishMs >= DiagnosticsPublishIntervalMs;

    if (mqttConnected && (diagnosticsRetryDue || diagnosticsUpdateDue))
    {
        publishDiagnostics(diagnosticsPublishPending);
    }

    updateTargetPositionMovement();
    publishPositionIfNeeded();
    publishCoverState();
}

void HomeAssistantBridge::publishPositionIfNeeded()
{
    unsigned long now = millis();
    int currentPosition = constrain(position.getPosition(), 0, 100);

    if (currentPosition == lastPublishedPosition)
    {
        return;
    }

    if (position.isMoving() &&
        now - lastPositionPublishMs < PositionPublishIntervalMs)
    {
        return;
    }

    publishPosition();
}

bool HomeAssistantBridge::publishPosition(bool force)
{
    int currentPosition = constrain(position.getPosition(), 0, 100);

    if (awningCover.setPosition(currentPosition, force))
    {
        lastPublishedPosition = currentPosition;
        lastPositionPublishMs = millis();
        return true;
    }

    return false;
}

void HomeAssistantBridge::publishFinalPosition()
{
    int currentPosition = constrain(position.getPosition(), 0, 100);

    if (publishPosition(true))
    {
        Logger::info("Final cover position published: " + String(currentPosition) + "%");
    }
    else
    {
        lastPublishedPosition = -1;
        Logger::warning(
            "Failed to publish final cover position " +
            String(currentPosition) +
            "%; retry scheduled");
    }
}

void HomeAssistantBridge::publishCoverState()
{
    publishCoverState(getCoverState());
}

void HomeAssistantBridge::publishCoverState(HACover::CoverState state, bool force)
{
    if (!force && state == lastPublishedState)
    {
        return;
    }

    if (awningCover.setState(state, force))
    {
        lastPublishedState = state;
    }
}

bool HomeAssistantBridge::publishDiagnostics(bool force)
{
    lastDiagnosticsPublishAttemptMs = millis();

    uint32_t uptimeSeconds = static_cast<uint32_t>(
        esp_timer_get_time() / 1000000ULL);
    int32_t wifiRssi = WiFi.RSSI();

    bool firmwarePublished = firmwareVersionSensor.setValue(Config::Device::Version);
    bool uptimePublished = uptimeSensor.setValue(uptimeSeconds, force);
    bool wifiRssiPublished = wifiRssiSensor.setValue(wifiRssi, force);

    if (firmwarePublished && uptimePublished && wifiRssiPublished)
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

void HomeAssistantBridge::subscribeRestartCommand()
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

HACover::CoverState HomeAssistantBridge::getCoverState() const
{
    if (position.isMoving())
    {
        if (position.getDirection() == MovementDirection::Opening)
        {
            return HACover::StateOpening;
        }

        if (position.getDirection() == MovementDirection::Closing)
        {
            return HACover::StateClosing;
        }
    }

    if (position.getPosition() <= 0)
    {
        return HACover::StateClosed;
    }

    if (position.getPosition() >= 100)
    {
        return HACover::StateOpen;
    }

    return HACover::StateStopped;
}

void HomeAssistantBridge::synchronizeMqttState()
{
    Logger::info("MQTT connected: synchronizing Home Assistant state");

    removeLegacyTargetPositionDiscovery();

    publishPosition(true);
    publishCoverState(getCoverState(), true);
    savedPositionAssumedPercentNumber.setState(savedPositionAssumedPercent, true);
    publishDiagnostics(true);
}

void HomeAssistantBridge::configureNativePositionMqtt()
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
    awningCover.setMqttConnectedCallback(onCoverMqttConnected);
}

void HomeAssistantBridge::setupCoverMqttConnection()
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

bool HomeAssistantBridge::publishCoverDiscovery()
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

void HomeAssistantBridge::removeLegacyTargetPositionDiscovery()
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

void HomeAssistantBridge::updateTargetPositionMovement()
{
    if (!targetPositionActive)
    {
        return;
    }

    int currentPosition = constrain(position.getPosition(), 0, 100);

    bool reachedTarget = abs(currentPosition - targetPosition) <= TargetPositionTolerance;

    if (position.getDirection() == MovementDirection::Opening && currentPosition >= targetPosition)
    {
        reachedTarget = true;
    }
    else if (position.getDirection() == MovementDirection::Closing && currentPosition <= targetPosition)
    {
        reachedTarget = true;
    }

    if (!reachedTarget)
    {
        return;
    }

    Logger::info("Target position reached: " + String(targetPosition) + "%");

    if (targetPositionRequiresPhysicalStop)
    {
        targetPositionActive = false;
        pendingMovementStartsTarget = false;
        remote.pressStop();
        return;
    }

    position.stop();

    targetPositionActive = false;

    publishFinalPosition();
    publishCoverState();
}

void HomeAssistantBridge::onCoverCommand(HACover::CoverCommand cmd, HACover *sender)
{
    if (instance == nullptr)
    {
        Logger::error("HomeAssistantBridge instance is null");
        return;
    }

    instance->handleCoverCommand(cmd);
}

void HomeAssistantBridge::onSavedPositionCommand(HAButton *sender)
{
    if (instance == nullptr)
    {
        Logger::error("HomeAssistantBridge instance is null");
        return;
    }

    instance->handleSavedPositionCommand();
}

void HomeAssistantBridge::onSavedPositionAssumedPercentCommand(
    HANumeric number,
    HANumber *sender)
{
    if (instance == nullptr)
    {
        Logger::error("HomeAssistantBridge instance is null");
        return;
    }

    instance->handleSavedPositionAssumedPercentCommand(number);
}

void HomeAssistantBridge::onRestartCommand(HAButton *sender)
{
    (void)sender;
    Logger::info("Home Assistant command: Restart");
    Serial.flush();
    ESP.restart();
}

void HomeAssistantBridge::onDiagnosticsMqttConnected()
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

void HomeAssistantBridge::onRemoteCommandStarted(RemoteController::Command command)
{
    if (instance != nullptr)
    {
        instance->handleRemoteCommandStarted(command);
    }
}

void HomeAssistantBridge::onCoverMqttConnected()
{
    if (instance != nullptr)
    {
        instance->setupCoverMqttConnection();
    }
}

void HomeAssistantBridge::onMqttMessage(
    const char *topic,
    const uint8_t *payload,
    uint16_t length)
{
    if (instance != nullptr)
    {
        instance->handleMqttMessage(topic, payload, length);
    }
}

void HomeAssistantBridge::handleMqttMessage(
    const char *topic,
    const uint8_t *payload,
    uint16_t length)
{
    if (strcmp(topic, coverPositionCommandTopic.c_str()) != 0)
    {
        return;
    }

    HANumeric number = HANumeric::fromStr(payload, length);

    if (!number.isSet())
    {
        Logger::warning("Home Assistant command: Invalid cover target position");
        return;
    }

    moveToTargetPosition(constrain(number.toInt16(), 0, 100));
}

void HomeAssistantBridge::handleCoverCommand(HACover::CoverCommand cmd)
{
    if (cmd == HACover::CommandOpen)
    {
        Logger::info("Home Assistant command: Open");
        targetPositionActive = false;
        pendingMovementStartsTarget = false;

        remote.pressOpen();
    }
    else if (cmd == HACover::CommandClose)
    {
        Logger::info("Home Assistant command: Close");
        targetPositionActive = false;
        pendingMovementStartsTarget = false;

        remote.pressClose();
    }
    else if (cmd == HACover::CommandStop)
    {
        Logger::info("Home Assistant command: Stop");
        targetPositionActive = false;
        pendingMovementStartsTarget = false;

        remote.pressStop();
    }
}

void HomeAssistantBridge::handleSavedPositionCommand()
{
    Logger::info("Home Assistant command: Saved position");
    targetPositionActive = false;
    pendingMovementStartsTarget = false;

    int currentPosition = constrain(position.getPosition(), 0, 100);
    int assumedPosition = savedPositionAssumedPercent;

    if (abs(currentPosition - assumedPosition) <= TargetPositionTolerance)
    {
        publishFinalPosition();
        publishCoverState();
        return;
    }

    remote.pressFavoritePosition();
}

void HomeAssistantBridge::handleSavedPositionAssumedPercentCommand(HANumeric number)
{
    if (!number.isSet())
    {
        Logger::warning("Home Assistant command: Invalid assumed saved position");
        return;
    }

    int requestedPosition = constrain(number.toInt16(), 0, 100);

    // This value only changes the firmware estimate; it does not program the motor favorite position.
    if (requestedPosition != savedPositionAssumedPercent)
    {
        savedPositionAssumedPercent = requestedPosition;

        if (preferencesReady &&
            preferences.putInt(SavedPositionPercentKey, savedPositionAssumedPercent) != sizeof(int))
        {
            Logger::warning("Failed to persist assumed saved position");
        }

        Logger::info(
            "Assumed saved position set to " +
            String(savedPositionAssumedPercent) +
            "%");
    }

    savedPositionAssumedPercentNumber.setState(savedPositionAssumedPercent, true);
}

void HomeAssistantBridge::loadSavedPositionAssumedPercent()
{
    savedPositionAssumedPercent = Config::Awning::SavedPositionPercent;
    preferencesReady = preferences.begin(PreferencesNamespace, false);

    if (!preferencesReady)
    {
        Logger::warning("Assumed saved-position persistence unavailable");
        return;
    }

    int storedPosition = preferences.getInt(
        SavedPositionPercentKey,
        Config::Awning::SavedPositionPercent);

    if (storedPosition >= 0 && storedPosition <= 100)
    {
        savedPositionAssumedPercent = storedPosition;
    }
    else
    {
        Logger::warning("Stored assumed saved position is invalid; using default");
    }
}

void HomeAssistantBridge::handleRemoteCommandStarted(RemoteController::Command command)
{
    leds.flashSend();

    if (command == RemoteController::Command::Open)
    {
        position.startOpening();
        targetPositionActive = pendingMovementStartsTarget;
        pendingMovementStartsTarget = false;
        publishCoverState();
    }
    else if (command == RemoteController::Command::Close)
    {
        position.startClosing();
        targetPositionActive = pendingMovementStartsTarget;
        pendingMovementStartsTarget = false;
        publishCoverState();
    }
    else if (command == RemoteController::Command::Stop)
    {
        targetPositionActive = false;
        pendingMovementStartsTarget = false;
        position.stop();
        publishFinalPosition();
        publishCoverState();
    }
    else if (command == RemoteController::Command::Favorite)
    {
        int currentPosition = constrain(position.getPosition(), 0, 100);
        int assumedPosition = savedPositionAssumedPercent;

        pendingMovementStartsTarget = false;
        targetPosition = assumedPosition;
        targetPositionRequiresPhysicalStop = false;

        if (abs(currentPosition - assumedPosition) <= TargetPositionTolerance)
        {
            targetPositionActive = false;
            position.stop();
            publishFinalPosition();
            publishCoverState();
        }
        else
        {
            targetPositionActive = true;

            if (currentPosition < assumedPosition)
            {
                position.startOpening();
            }
            else
            {
                position.startClosing();
            }

            publishCoverState();
        }
    }
}

void HomeAssistantBridge::moveToTargetPosition(int requestedPosition)
{
    int currentPosition = constrain(position.getPosition(), 0, 100);

    Logger::info(
        "Home Assistant command: Target position " +
        String(requestedPosition) +
        "% from " +
        String(currentPosition) +
        "%");

    if (abs(currentPosition - requestedPosition) <= TargetPositionTolerance)
    {
        targetPositionActive = false;

        if (position.isMoving())
        {
            targetPositionActive = false;
            pendingMovementStartsTarget = false;
            remote.pressStop();
            return;
        }

        publishFinalPosition();
        publishCoverState();
        return;
    }

    targetPosition = requestedPosition;
    targetPositionRequiresPhysicalStop = true;
    targetPositionActive = false;
    pendingMovementStartsTarget = true;

    if (requestedPosition > currentPosition)
    {
        remote.pressOpen();
    }
    else
    {
        remote.pressClose();
    }
}

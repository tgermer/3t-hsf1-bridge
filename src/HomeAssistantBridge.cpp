#include "HomeAssistantBridge.h"

#include "Config.h"
#include "Logger.h"

namespace
{
    constexpr auto PreferencesNamespace = "ha-bridge";
    constexpr auto SavedPositionPercentKey = "savedPercent";
}

HomeAssistantBridge *HomeAssistantBridge::instance = nullptr;

HomeAssistantBridge::HomeAssistantBridge(
    WiFiManager &wifiManager,
    MQTTManager &mqttManager,
    RemoteController &remote,
    PositionTracker &position,
    LedController &leds)
    : wifiManager(wifiManager),
      mqttManager(mqttManager),
      remote(remote),
      position(position),
      leds(leds),
      awningCover("markise", HACover::PositionFeature),
      savedPositionButton("markise_saved_position"),
      savedPositionAssumedPercentNumber("markise_saved_position_assumed_percent"),
      diagnostics(wifiManager, mqttManager),
      nativeCoverMqtt(mqttManager, awningCover)
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

    diagnostics.begin();
    nativeCoverMqtt.begin(onNativeSetPositionCommand);

    Logger::info(
        "Home Assistant cover setup complete: native position support enabled");
}

void HomeAssistantBridge::update()
{
    bool mqttConnected = mqttManager.isConnected();

    nativeCoverMqtt.update();

    if (mqttConnected && !lastMqttConnected)
    {
        synchronizeMqttState();
    }

    lastMqttConnected = mqttConnected;
    diagnostics.update();

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

    nativeCoverMqtt.synchronize();

    publishPosition(true);
    publishCoverState(getCoverState(), true);
    savedPositionAssumedPercentNumber.setState(savedPositionAssumedPercent, true);
    diagnostics.synchronize();
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

void HomeAssistantBridge::onRemoteCommandStarted(RemoteController::Command command)
{
    if (instance != nullptr)
    {
        instance->handleRemoteCommandStarted(command);
    }
}

void HomeAssistantBridge::onNativeSetPositionCommand(
    const uint8_t *payload,
    uint16_t length)
{
    if (instance != nullptr)
    {
        instance->handleNativeSetPositionCommand(payload, length);
    }
}

void HomeAssistantBridge::handleNativeSetPositionCommand(
    const uint8_t *payload,
    uint16_t length)
{
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

#include "HomeAssistantBridge.h"

#include "Config.h"
#include "Logger.h"

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
      targetPositionNumber("markise_target_position")
{
}

void HomeAssistantBridge::begin()
{
    Logger::info("Initializing Home Assistant bridge");

    instance = this;

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

    targetPositionNumber.setName("Zielposition");
    targetPositionNumber.setIcon("mdi:arrow-expand-vertical");
    targetPositionNumber.setMin(0);
    targetPositionNumber.setMax(100);
    targetPositionNumber.setStep(1);
    targetPositionNumber.setMode(HANumber::ModeSlider);
    targetPositionNumber.onCommand(onTargetPositionCommand);
    targetPositionNumber.setState(position.getPosition());

    Logger::info("Home Assistant cover registered");
}

void HomeAssistantBridge::update()
{
    bool mqttConnected = mqttManager.isConnected();

    if (mqttConnected && !lastMqttConnected)
    {
        synchronizeMqttState();
    }

    lastMqttConnected = mqttConnected;

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

void HomeAssistantBridge::publishPosition(bool force)
{
    int currentPosition = constrain(position.getPosition(), 0, 100);

    if (awningCover.setPosition(currentPosition, force))
    {
        lastPublishedPosition = currentPosition;
        lastPositionPublishMs = millis();
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

    publishPosition(true);
    publishCoverState(getCoverState(), true);

    if (targetPositionNumber.getCurrentState().isSet())
    {
        targetPositionNumber.setState(targetPositionNumber.getCurrentState(), true);
    }
    else
    {
        targetPositionNumber.setState(position.getPosition(), true);
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
        leds.flashSend();
        remote.pressStop();
    }

    position.stop();

    targetPositionActive = false;

    publishPosition(true);
    publishCoverState();
    targetPositionNumber.setState(targetPosition);
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

void HomeAssistantBridge::onTargetPositionCommand(HANumeric number, HANumber *sender)
{
    if (instance == nullptr)
    {
        Logger::error("HomeAssistantBridge instance is null");
        return;
    }

    instance->handleTargetPositionCommand(number);
}

void HomeAssistantBridge::handleCoverCommand(HACover::CoverCommand cmd)
{
    if (cmd == HACover::CommandOpen)
    {
        Logger::info("Home Assistant command: Open");
        targetPositionActive = false;

        leds.flashSend();
        remote.pressOpen();
        position.startOpening();

        publishCoverState();
    }
    else if (cmd == HACover::CommandClose)
    {
        Logger::info("Home Assistant command: Close");
        targetPositionActive = false;

        leds.flashSend();
        remote.pressClose();
        position.startClosing();

        publishCoverState();
    }
    else if (cmd == HACover::CommandStop)
    {
        Logger::info("Home Assistant command: Stop");
        targetPositionActive = false;

        position.stop();

        leds.flashSend();
        remote.pressStop();

        publishPosition(true);
        publishCoverState();
    }
}

void HomeAssistantBridge::handleSavedPositionCommand()
{
    Logger::info("Home Assistant command: Saved position");
    targetPositionActive = false;

    int currentPosition = constrain(position.getPosition(), 0, 100);

    if (abs(currentPosition - Config::Awning::SavedPositionPercent) <= TargetPositionTolerance)
    {
        publishPosition(true);
        publishCoverState();
        return;
    }

    leds.flashSend();
    remote.pressFavoritePosition();

    targetPosition = Config::Awning::SavedPositionPercent;
    targetPositionRequiresPhysicalStop = false;
    targetPositionActive = true;

    if (currentPosition < Config::Awning::SavedPositionPercent)
    {
        position.startOpening();
    }
    else
    {
        position.startClosing();
    }

    publishCoverState();
}

void HomeAssistantBridge::handleTargetPositionCommand(HANumeric number)
{
    if (!number.isSet())
    {
        Logger::warning("Home Assistant command: Target position reset ignored");
        return;
    }

    int requestedPosition = constrain(number.toInt16(), 0, 100);
    int currentPosition = constrain(position.getPosition(), 0, 100);

    Logger::info(
        "Home Assistant command: Target position " +
        String(requestedPosition) +
        "% from " +
        String(currentPosition) +
        "%");

    targetPositionNumber.setState(requestedPosition);

    if (abs(currentPosition - requestedPosition) <= TargetPositionTolerance)
    {
        targetPositionActive = false;

        if (position.isMoving())
        {
            leds.flashSend();
            remote.pressStop();
            position.stop();
        }

        publishPosition(true);
        publishCoverState();
        return;
    }

    leds.flashSend();

    targetPosition = requestedPosition;
    targetPositionRequiresPhysicalStop = true;
    targetPositionActive = true;

    if (requestedPosition > currentPosition)
    {
        remote.pressOpen();
        position.startOpening();
        publishCoverState();
    }
    else
    {
        remote.pressClose();
        position.startClosing();
        publishCoverState();
    }
}

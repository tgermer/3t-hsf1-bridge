#include "HomeAssistantBridge.h"

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
      savedPositionButton("markise_saved_position")
{
}

void HomeAssistantBridge::begin()
{
    Logger::info("Initializing Home Assistant bridge");

    instance = this;

    awningCover.setName("Markise");
    awningCover.setDeviceClass("awning");
    awningCover.setIcon("mdi:awning-outline");
    awningCover.setOptimistic(true);
    awningCover.onCommand(onCoverCommand);
    awningCover.setPosition(position.getPosition());
    awningCover.setState(HACover::StateClosed);

    savedPositionButton.setName("Gespeicherte Position");
    savedPositionButton.setIcon("mdi:star-outline");
    savedPositionButton.onCommand(onSavedPositionCommand);

    lastPublishedPosition = position.getPosition();

    Logger::info("Home Assistant cover registered");
}

void HomeAssistantBridge::update()
{
    // Temporarily disabled to isolate the MQTT keepalive disconnect issue.
    // Re-enable publishPositionIfNeeded() after the MQTT connection is stable.
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

    awningCover.setPosition(currentPosition);
    publishCoverState();

    lastPublishedPosition = currentPosition;
    lastPositionPublishMs = now;
}

void HomeAssistantBridge::publishCoverState()
{
    if (position.isMoving())
    {
        if (position.getDirection() == MovementDirection::Opening)
        {
            awningCover.setState(HACover::StateOpening);
        }
        else if (position.getDirection() == MovementDirection::Closing)
        {
            awningCover.setState(HACover::StateClosing);
        }

        return;
    }

    if (position.getPosition() <= 0)
    {
        awningCover.setState(HACover::StateClosed);
    }
    else if (position.getPosition() >= 100)
    {
        awningCover.setState(HACover::StateOpen);
    }
    else
    {
        awningCover.setState(HACover::StateStopped);
    }
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

void HomeAssistantBridge::handleCoverCommand(HACover::CoverCommand cmd)
{
    if (cmd == HACover::CommandOpen)
    {
        Logger::info("Home Assistant command: Open");

        leds.flashSend();
        remote.pressOpen();
        position.startOpening();

        awningCover.setState(HACover::StateOpening);
    }
    else if (cmd == HACover::CommandClose)
    {
        Logger::info("Home Assistant command: Close");

        leds.flashSend();
        remote.pressClose();
        position.startClosing();

        awningCover.setState(HACover::StateClosing);
    }
    else if (cmd == HACover::CommandStop)
    {
        Logger::info("Home Assistant command: Stop");

        leds.flashSend();
        remote.pressStop();
        position.stop();

        awningCover.setState(HACover::StateStopped);
    }
}

void HomeAssistantBridge::handleSavedPositionCommand()
{
    Logger::info("Home Assistant command: Saved position");

    leds.flashSend();
    remote.pressFavoritePosition();

    awningCover.setState(HACover::StateStopped);
}
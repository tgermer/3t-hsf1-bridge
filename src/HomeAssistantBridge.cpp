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
      awningCover("markise")
{
}

void HomeAssistantBridge::begin()
{
    Logger::info("Initializing Home Assistant bridge");

    instance = this;

    awningCover.setName("Markise");
    awningCover.setDeviceClass("awning");
    awningCover.setIcon("mdi:awning");
    awningCover.setOptimistic(true);
    awningCover.onCommand(onCoverCommand);
    awningCover.setCurrentState(HACover::StateUnknown);

    Logger::info("Home Assistant cover registered");
}

void HomeAssistantBridge::update()
{
    // Reserved for future Home Assistant updates.
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

void HomeAssistantBridge::handleCoverCommand(HACover::CoverCommand cmd)
{
    if (cmd == HACover::CommandOpen)
    {
        Logger::info("Home Assistant command: Open");

        leds.flashSend();
        remote.pressOpen();
        position.startOpening();

        awningCover.setState(HACover::StateOpen);
    }
    else if (cmd == HACover::CommandClose)
    {
        Logger::info("Home Assistant command: Close");

        leds.flashSend();
        remote.pressClose();
        position.startClosing();

        awningCover.setState(HACover::StateClosed);
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
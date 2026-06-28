#pragma once

#include <ArduinoHA.h>

#include "LedController.h"
#include "MQTTManager.h"
#include "PositionTracker.h"
#include "RemoteController.h"

class HomeAssistantBridge
{
public:
    HomeAssistantBridge(
        MQTTManager &mqttManager,
        RemoteController &remote,
        PositionTracker &position,
        LedController &leds);

    void begin();
    void update();

private:
    MQTTManager &mqttManager;
    RemoteController &remote;
    PositionTracker &position;
    LedController &leds;

    HACover awningCover;
    HAButton savedPositionButton;
    HANumber targetPositionNumber;

    int lastPublishedPosition = -1;
    HACover::CoverState lastPublishedState = HACover::StateUnknown;
    unsigned long lastPositionPublishMs = 0;
    bool lastMqttConnected = false;
    bool targetPositionActive = false;
    bool targetPositionRequiresPhysicalStop = true;
    int targetPosition = -1;

    static constexpr unsigned long PositionPublishIntervalMs = 1000;
    static constexpr int TargetPositionTolerance = 1;

    static HomeAssistantBridge *instance;
    static void onCoverCommand(HACover::CoverCommand cmd, HACover *sender);
    static void onSavedPositionCommand(HAButton *sender);
    static void onTargetPositionCommand(HANumeric number, HANumber *sender);

    void publishPositionIfNeeded();
    void publishPosition(bool force = false);
    void publishCoverState();
    void publishCoverState(HACover::CoverState state, bool force = false);
    HACover::CoverState getCoverState() const;
    void synchronizeMqttState();
    void updateTargetPositionMovement();

    void handleCoverCommand(HACover::CoverCommand cmd);
    void handleSavedPositionCommand();
    void handleTargetPositionCommand(HANumeric number);
};

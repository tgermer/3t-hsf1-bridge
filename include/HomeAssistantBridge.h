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
    unsigned long lastPositionPublishMs = 0;
    bool targetPositionActive = false;
    int targetPosition = -1;

    static constexpr unsigned long PositionPublishIntervalMs = 1000;
    static constexpr int TargetPositionTolerance = 1;

    static HomeAssistantBridge *instance;
    static void onCoverCommand(HACover::CoverCommand cmd, HACover *sender);
    static void onSavedPositionCommand(HAButton *sender);
    static void onTargetPositionCommand(HANumeric number, HANumber *sender);

    void publishPositionIfNeeded();
    void publishCoverState();
    void updateTargetPositionMovement();

    void handleCoverCommand(HACover::CoverCommand cmd);
    void handleSavedPositionCommand();
    void handleTargetPositionCommand(HANumeric number);
};
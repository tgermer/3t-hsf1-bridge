#pragma once

#include "HomeAssistantBridge.h"
#include "LedController.h"
#include "MQTTManager.h"
#include "PositionTracker.h"
#include "RemoteController.h"
#include "WiFiManager.h"

class Application
{
public:
    Application();

    void begin();
    void update();

private:
    LedController leds;
    RemoteController remote;
    PositionTracker position;
    WiFiManager wifi;
    MQTTManager mqtt;
    HomeAssistantBridge bridge;
};
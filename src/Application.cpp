#include "Application.h"

#include "Logger.h"

Application::Application()
    : bridge(mqtt, remote, position, leds)
{
}

void Application::begin()
{
    Logger::begin();
    Logger::info("3T HSF1 Bridge starting");

    leds.begin();
    remote.begin();
    position.begin();

    wifi.begin();
    mqtt.begin();
    bridge.begin();
}

void Application::update()
{
    wifi.update();

    if (wifi.isConnected() && !mqtt.isStarted())
    {
        mqtt.connect();
    }

    mqtt.update();
    bridge.update();
    position.update();
    leds.update();
    remote.update();

    leds.setWifi(wifi.isConnected());
    leds.setMqtt(mqtt.isConnected());
}
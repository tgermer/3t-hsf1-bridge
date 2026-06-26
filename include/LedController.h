#pragma once

class LedController
{
public:
    void begin();

    void setWifi(bool connected);
    void setMqtt(bool connected);
    void setError(bool error);

    void flashSend();

private:
    bool wifiState = false;
    bool mqttState = false;
    bool errorState = false;
};
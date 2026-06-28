#pragma once

#include <Arduino.h>

class RemoteController
{
public:
    enum class Command : uint8_t
    {
        Open,
        Stop,
        Close,
        Favorite
    };

    using CommandStartedCallback = void (*)(Command command);

    void begin();
    void update();

    void pressOpen();
    void pressStop();
    void pressClose();
    void pressFavoritePosition();
    void onCommandStarted(CommandStartedCallback callback);

private:
    struct ButtonRequest
    {
        Command command = Command::Stop;
        uint8_t pin = 0;
        uint32_t durationMs = 0;
    };

    bool buttonActive = false;
    uint8_t activePin = 0;
    unsigned long buttonStartedAt = 0;
    uint32_t buttonDurationMs = 0;
    bool pendingRequestAvailable = false;
    ButtonRequest pendingRequest;
    CommandStartedCallback commandStartedCallback = nullptr;

    void queueButton(Command command, uint8_t pin, uint32_t durationMs);
    void startButton(const ButtonRequest &request);
};

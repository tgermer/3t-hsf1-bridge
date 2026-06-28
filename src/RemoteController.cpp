#include "RemoteController.h"
#include "Pins.h"

#include "Config.h"

void RemoteController::begin()
{
    pinMode(Pins::Remote::Open, OUTPUT);
    pinMode(Pins::Remote::Stop, OUTPUT);
    pinMode(Pins::Remote::Close, OUTPUT);

    digitalWrite(Pins::Remote::Open, LOW);
    digitalWrite(Pins::Remote::Stop, LOW);
    digitalWrite(Pins::Remote::Close, LOW);

    buttonActive = false;
    pendingRequestAvailable = false;
}

void RemoteController::update()
{
    if (!buttonActive)
    {
        return;
    }

    if (millis() - buttonStartedAt >= buttonDurationMs)
    {
        digitalWrite(activePin, LOW);
        buttonActive = false;

        if (pendingRequestAvailable)
        {
            ButtonRequest request = pendingRequest;
            pendingRequestAvailable = false;
            startButton(request);
        }
    }
}

void RemoteController::pressOpen()
{
    queueButton(Command::Open, Pins::Remote::Open, Config::Remote::ButtonPressMs);
}

void RemoteController::pressStop()
{
    queueButton(Command::Stop, Pins::Remote::Stop, Config::Remote::ButtonPressMs);
}

void RemoteController::pressClose()
{
    queueButton(Command::Close, Pins::Remote::Close, Config::Remote::ButtonPressMs);
}

void RemoteController::pressFavoritePosition()
{
    queueButton(Command::Favorite, Pins::Remote::Stop, Config::Remote::LongButtonPressMs);
}

void RemoteController::onCommandStarted(CommandStartedCallback callback)
{
    commandStartedCallback = callback;
}

void RemoteController::queueButton(Command command, uint8_t pin, uint32_t durationMs)
{
    ButtonRequest request;
    request.command = command;
    request.pin = pin;
    request.durationMs = durationMs;

    if (!buttonActive)
    {
        startButton(request);
        return;
    }

    // Never interrupt the active pulse. Keeping only the newest pending command
    // avoids executing stale movement commands after the user's intent has changed.
    pendingRequest = request;
    pendingRequestAvailable = true;
}

void RemoteController::startButton(const ButtonRequest &request)
{
    activePin = request.pin;
    buttonDurationMs = request.durationMs;
    buttonStartedAt = millis();
    buttonActive = true;

    digitalWrite(activePin, HIGH);

    if (commandStartedCallback != nullptr)
    {
        commandStartedCallback(request.command);
    }
}

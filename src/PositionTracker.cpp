#include "PositionTracker.h"
#include "Config.h"

#include "Logger.h"

namespace
{
    constexpr auto PreferencesNamespace = "awning";
    constexpr auto PositionKey = "position";
    constexpr auto PositionValidKey = "valid";
}

void PositionTracker::begin()
{
    position = 0.0f;
    positionValid = false;
    direction = MovementDirection::Idle;
    lastUpdateMs = millis();

    preferencesReady = preferences.begin(PreferencesNamespace, false);
    loadPosition();

    Logger::info(
        "Position tracker initialized at " +
        String(getPosition()) +
        "% (valid: " +
        String(positionValid ? "yes" : "no") +
        ")");
}

void PositionTracker::startOpening()
{
    update();

    direction = MovementDirection::Opening;
    lastUpdateMs = millis();

    Logger::info("Position tracking: opening");
}

void PositionTracker::startClosing()
{
    update();

    direction = MovementDirection::Closing;
    lastUpdateMs = millis();

    Logger::info("Position tracking: closing");
}

void PositionTracker::stop()
{
    update();

    direction = MovementDirection::Idle;
    positionValid = true;

    Logger::info("Position tracking stopped at " + String(getPosition()) + "%");
    savePosition();
}

void PositionTracker::setFullyOpen()
{
    position = 100.0f;
    direction = MovementDirection::Idle;
    positionValid = true;

    Logger::info("Position set to fully open");
    savePosition();
}

void PositionTracker::setFullyClosed()
{
    position = 0.0f;
    direction = MovementDirection::Idle;
    positionValid = true;

    Logger::info("Position set to fully closed");
    savePosition();
}

void PositionTracker::update()
{
    if (direction == MovementDirection::Idle)
    {
        return;
    }

    unsigned long now = millis();
    unsigned long elapsedMs = now - lastUpdateMs;

    if (elapsedMs == 0)
    {
        return;
    }

    applyMovement(elapsedMs);
    lastUpdateMs = now;
}

int PositionTracker::getPosition() const
{
    return static_cast<int>(round(position));
}

MovementDirection PositionTracker::getDirection() const
{
    return direction;
}

bool PositionTracker::isMoving() const
{
    return direction != MovementDirection::Idle;
}

bool PositionTracker::isPositionValid() const
{
    return positionValid;
}

void PositionTracker::applyMovement(unsigned long elapsedMs)
{
    if (direction == MovementDirection::Opening)
    {
        float delta = (elapsedMs * 100.0f) / Config::Awning::OpenTimeMs;
        position += delta;

        if (position >= 100)
        {
            position = 100.0f;
            direction = MovementDirection::Idle;
            positionValid = true;
            Logger::info("Position reached fully open");
            savePosition();
        }
    }
    else if (direction == MovementDirection::Closing)
    {
        float delta = (elapsedMs * 100.0f) / Config::Awning::CloseTimeMs;
        position -= delta;

        if (position <= 0)
        {
            position = 0.0f;
            direction = MovementDirection::Idle;
            positionValid = true;
            Logger::info("Position reached fully closed");
            savePosition();
        }
    }
}

void PositionTracker::loadPosition()
{
    if (!preferencesReady)
    {
        Logger::warning("Position persistence unavailable");
        return;
    }

    bool hasPosition = preferences.isKey(PositionKey);
    bool hasPositionValid = preferences.isKey(PositionValidKey);
    int storedPosition = preferences.getInt(PositionKey, 0);
    bool storedPositionValid = preferences.getBool(PositionValidKey, false);

    lastPersistedPosition = hasPosition ? storedPosition : -1;
    lastPersistedPositionValid = hasPositionValid ? static_cast<int>(storedPositionValid) : -1;

    if (hasPosition && hasPositionValid && storedPositionValid &&
        storedPosition >= 0 && storedPosition <= 100)
    {
        position = static_cast<float>(storedPosition);
        positionValid = true;
    }
}

void PositionTracker::savePosition()
{
    if (!preferencesReady)
    {
        return;
    }

    int currentPosition = getPosition();
    bool positionSaved = true;

    if (currentPosition != lastPersistedPosition)
    {
        positionSaved = preferences.putInt(PositionKey, currentPosition) == sizeof(int);

        if (positionSaved)
        {
            lastPersistedPosition = currentPosition;
        }
        else
        {
            Logger::warning("Failed to persist awning position");
        }
    }

    int currentPositionValid = positionValid ? 1 : 0;

    if (positionSaved && currentPositionValid != lastPersistedPositionValid)
    {
        if (preferences.putBool(PositionValidKey, positionValid) == sizeof(bool))
        {
            lastPersistedPositionValid = currentPositionValid;
        }
        else
        {
            Logger::warning("Failed to persist awning position validity");
        }
    }
}

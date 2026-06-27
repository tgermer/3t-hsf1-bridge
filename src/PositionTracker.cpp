#include "PositionTracker.h"
#include "Config.h"

#include "Logger.h"

void PositionTracker::begin()
{
    position = 0.0f;
    direction = MovementDirection::Idle;
    lastUpdateMs = millis();

    Logger::info("Position tracker initialized");
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

    Logger::info("Position tracking stopped at " + String(getPosition()) + "%");
}

void PositionTracker::setFullyOpen()
{
    position = 100.0f;
    direction = MovementDirection::Idle;

    Logger::info("Position set to fully open");
}

void PositionTracker::setFullyClosed()
{
    position = 0.0f;
    direction = MovementDirection::Idle;

    Logger::info("Position set to fully closed");
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
            Logger::info("Position reached fully open");
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
            Logger::info("Position reached fully closed");
        }
    }
}
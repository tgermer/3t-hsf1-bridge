#pragma once

#include <Arduino.h>

enum class MovementDirection
{
    Idle,
    Opening,
    Closing
};

class PositionTracker
{
public:
    void begin();

    void startOpening();
    void startClosing();
    void stop();

    void setFullyOpen();
    void setFullyClosed();

    void update();

    int getPosition() const;
    MovementDirection getDirection() const;
    bool isMoving() const;

private:
    int position = 0; // 0 = eingefahren, 100 = ausgefahren
    MovementDirection direction = MovementDirection::Idle;
    unsigned long lastUpdateMs = 0;

    void applyMovement(unsigned long elapsedMs);
};
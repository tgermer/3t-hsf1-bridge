#pragma once

#include <Arduino.h>
#include <Preferences.h>

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
    bool isPositionValid() const;

private:
    float position = 0.0f; // 0 = eingefahren, 100 = ausgefahren
    MovementDirection direction = MovementDirection::Idle;
    unsigned long lastUpdateMs = 0;
    Preferences preferences;
    bool preferencesReady = false;
    bool positionValid = false;
    int lastPersistedPosition = -1;
    int lastPersistedPositionValid = -1;

    void applyMovement(unsigned long elapsedMs);
    void loadPosition();
    void savePosition();
};

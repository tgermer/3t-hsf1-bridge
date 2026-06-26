# Logger

## Responsibility

Provides a central logging interface.

## Why

The application should never use `Serial.println()` directly.

## Public API

- info()
- warning()
- error()
- debug()

## Future ideas

- MQTT logging
- File logging
- Log levels

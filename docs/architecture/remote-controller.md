# RemoteController

## Responsibility

The `RemoteController` controls the electrical outputs that simulate button presses on the original HSF1 remote.

It does not know anything about Home Assistant, MQTT, WiFi, or position tracking.

## Why it exists

The HSF1 remote is used as the original RF transmitter.  
Instead of reproducing the 433 MHz signal directly, the ESP32 activates optocouplers that electrically press the HSF1 buttons.

This keeps the RF behavior identical to the original remote.

## Controlled buttons

| Method                    | HSF1 Button | Function                         |
| ------------------------- | ----------- | -------------------------------- |
| `pressOpen()`             | S1          | Extend awning                    |
| `pressStop()`             | S2          | Stop movement                    |
| `pressClose()`            | S3          | Retract awning                   |
| `pressFavoritePosition()` | S2          | Long press for favorite position |

## Design decision

GPIO handling is isolated inside this class.

Other parts of the application should not call `digitalWrite()` for HSF1 buttons directly.

## Future ideas

- Make button press duration configurable.
- Add non-blocking button presses.
- Add diagnostics for active channels.

# LedController

## Responsibility

The `LedController` controls the status LEDs of the bridge.

It provides a small API for setting system states without exposing GPIO details to the rest of the application.

## LEDs

| Method        | LED    | Meaning                        |
| ------------- | ------ | ------------------------------ |
| `setWifi()`   | Green  | WiFi connection state          |
| `setMqtt()`   | Blue   | MQTT connection state          |
| `flashSend()` | Yellow | A remote command is being sent |
| `setError()`  | Red    | Error or not-ready state       |

## Why it exists

LED GPIO handling should stay in one place.

The rest of the application should express intent:

```cpp
leds.setWifi(true);
leds.flashSend();
```

instead of directly writing to GPIO pins.

## Design decision

The first implementation is simple and blocking. This is acceptable for short LED flashes.

## Future ideas

- Non-blocking LED blinking.
- Different blink patterns for WiFi reconnect, MQTT reconnect, and errors.
- Heartbeat LED.
- Startup animation.

# PositionTracker

## Responsibility

The `PositionTracker` estimates the current awning position based on runtime.

It does not control GPIOs, Home Assistant, MQTT, LEDs, or the HSF1 remote.

## Position model

Home Assistant cover convention:

- `0%` = fully closed / fully retracted
- `100%` = fully open / fully extended

For this project:

- `0%` = awning fully retracted
- `100%` = awning fully extended

## Measured runtimes

| Direction       | Runtime |
| --------------- | ------: |
| Extend / Open   |  26.3 s |
| Retract / Close |  27.0 s |

## How it works

When movement starts, the tracker stores the current time and direction.

During `update()`, it calculates how much time has passed and converts that into a position change.

When the awning reaches a calculated end position, the tracker clamps the value:

- Opening reaches `100%`
- Closing reaches `0%`

## Why methods call `update()` first

Methods like `stop()`, `startOpening()`, and `startClosing()` call `update()` before changing state.

This ensures the tracker first calculates the current estimated position before stopping or changing direction.

Example:

1. Position is `0%`.
2. Awning starts opening.
3. After 5 seconds, `stop()` is called.
4. `stop()` first calls `update()`.
5. The position is calculated before movement is set to idle.

## Limitations

This is an estimated position.

If the awning is moved with the original HSF1 remote outside the ESP32 bridge, the bridge will not know about that movement.

## Calibration

The bridge should provide calibration actions:

- Set position to fully retracted (`0%`)
- Set position to fully extended (`100%`)

A full open or close command issued by the bridge can also be used to re-synchronize the estimated position.

## Future ideas

- Persist position in flash memory.
- Add Home Assistant calibration buttons.
- Add travel time configuration.
- Add manual correction from Home Assistant.

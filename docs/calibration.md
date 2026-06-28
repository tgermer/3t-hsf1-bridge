# Position Calibration

## Purpose

The motor does not report its position. The bridge estimates the awning position from the elapsed motor runtime, so accurate opening and closing travel times are required. Calibration also determines the percentage used for the motor's saved/favorite position.

Home Assistant uses the following convention:

- `0%` = fully closed/retracted
- `100%` = fully open/extended

## Starting conditions

Before measuring:

- Verify that OPEN, CLOSE, STOP, and the saved-position command work reliably.
- Ensure the motor's physical end stops and favorite position are already programmed.
- Do not use another physical remote during a measurement.
- Start each measurement at the specified physical end stop.
- Measure motor movement, not network or Home Assistant response time.
- Repeat each measurement several times and use the average if results differ.

## Measure full opening time

1. Move the awning to the fully closed physical end stop.
2. Start timing when the motor begins opening.
3. Send OPEN and allow the motor to run without interruption.
4. Stop timing when the motor stops at the fully open end stop.
5. Convert the measured seconds to milliseconds.

Example: `26.3 s` becomes `26300 ms`.

## Measure full closing time

1. Move the awning to the fully open physical end stop.
2. Start timing when the motor begins closing.
3. Send CLOSE and allow the motor to run without interruption.
4. Stop timing when the motor stops at the fully closed end stop.
5. Convert the measured seconds to milliseconds.

Example: `27.0 s` becomes `27000 ms`.

## Calculate the saved position

Measure the saved-position command from the fully closed end stop. The command includes the long button press, which must be removed from the measured total time:

```text
movement time = total saved-position time - long button press time
saved position % = movement time / full opening time * 100
```

Example:

```text
total saved-position time = 21.5 s
LongButtonPressMs         =  3.0 s
movement time             = 18.5 s
full opening time         = 26.3 s

SavedPositionPercent = 18.5 / 26.3 * 100
                     = 70.3%
                     ≈ 70%
```

Round the result to the nearest whole percentage.

## Configure the firmware

Set the measured values in `include/Config.h`:

```cpp
namespace Remote
{
    constexpr uint32_t LongButtonPressMs = 3000;
}

namespace Awning
{
    constexpr uint32_t OpenTimeMs = 26300;
    constexpr uint32_t CloseTimeMs = 27000;
    constexpr int SavedPositionPercent = 70;
}
```

Rebuild and flash the firmware after changing these values.

## Verify in Home Assistant

After flashing:

1. Run a complete CLOSE movement and verify position `0%` and state `closed`.
2. Run a complete OPEN movement and verify that position increases to `100%` and state becomes `open`.
3. Stop midway and verify that the reported position remains near the physical position.
4. Move to several target positions, such as `25%`, `50%`, and `75%`.
5. Run the saved-position command and verify that it finishes near `70%`.
6. Confirm that the target-position number follows the reported cover position.

## Accuracy and recalibration

Position is time-based and remains an estimate. Motor response delay, radio latency, load, wind, supply voltage, and integer rounding can cause a deviation of a few percentage points. If the error is consistently directional, repeat the runtime measurements and adjust the configured value.

A complete uninterrupted OPEN or CLOSE movement reaches a known physical end stop and recalibrates the estimate to `100%` or `0%`. Use a full movement whenever the estimated and physical positions no longer agree.

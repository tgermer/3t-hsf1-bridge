# Runtime Measurements

## Awning

Motor: 3T45-40RB

Measured with stopwatch.

| Direction       | Runtime |
| --------------- | ------- |
| Extend (Open)   | 26.3 s  |
| Retract (Close) | 27.0 s  |

## Home Assistant Mapping

- 0% = Fully retracted
- 100% = Fully extended

## Position Tracking

The bridge estimates the current position based on the measured runtime.

```
position += elapsed / 26.3 s
position -= elapsed / 27.0 s
```

A complete open or close command resets the calculated position to 100% or 0%.

# RF Analysis

## Hardware

- ESP32 LOLIN32
- CC1101
- RF Sniffer

## Result

The CC1101 successfully received and replayed RF telegrams.

Observed:

- Device ID remained constant.
- Open command remained constant.
- Close command remained constant.
- Stop command uses a different protocol.

Despite transmitting matching telegrams, the 3T45-40RB motor did not react.

## Conclusion

Direct RF replay is currently considered unreliable.

The project therefore controls the original HSF1 remote electronically via optocouplers.

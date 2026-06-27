# Configuration

## Goal

Provide a single place for all configurable values used by the firmware.

## Structure

The configuration is grouped into logical namespaces:

- `Config::WiFi`
- `Config::MQTT`
- `Config::Device`
- `Config::Remote`
- `Config::Awning`

## Design Decision

Configuration values are grouped by responsibility instead of using global constants.

Example:

```cpp
Config::WiFi::SSID
Config::MQTT::Host
Config::Remote::ButtonPressMs
Config::Awning::OpenTimeMs
```

This improves readability and scalability as the project grows.

## Not included

Hardware pin assignments.

Those belong in Pins.h.

Update CHANGELOG.md for the v1.1.0 release.

Context: The project has evolved from an initial prototype into a stable Home Assistant integration.

Update the changelog using the "Keep a Changelog" format.

Add a new release:

## [1.1.0] - 2026-06-28

Include the following sections where appropriate:

### Added

- Native Home Assistant cover position support (`cover.set_cover_position`)
- Saved position button
- Configurable assumed saved-position percentage
- Position persistence using ESP32 Preferences (NVS)
- MQTT availability (online/offline)
- MQTT reconnect state synchronization
- Advanced Home Assistant diagnostics:
    - WiFi RSSI
    - WiFi SSID
    - IP address
    - MAC address
    - Free heap
    - Reset reason
    - WiFi reconnect counter
    - MQTT reconnect counter
    - Restart button
- Calibration documentation

### Changed

- Position tracking now starts when the physical remote pulse starts.
- Remote button presses are serialized.
- Native cover position control replaces the legacy target-position number.
- Saved-position tracking uses the configured assumed percentage.
- Cover state and position synchronization were reworked for higher reliability.
- Discovery and reconnect handling improved.

### Fixed

- MQTT optimistic state race conditions.
- STOP state/position synchronization.
- Position publication reliability.
- Endpoint state updates.
- MQTT reconnect synchronization.
- Startup state synchronization.
- Saved-position tracking.
- Native position command handling.
- Diagnostics synchronization.

### Removed

- Legacy target-position number entity.
- Firmware-version diagnostic entity (covered by device information).
- Boot timestamp diagnostics.

Keep previous release history intact. Do not modify anything else.

# MQTTManager

## Responsibility

The `MQTTManager` owns the MQTT connection used by the Home Assistant integration.

It is responsible for:

- configuring the Home Assistant MQTT device
- connecting to the MQTT broker
- keeping the MQTT client loop running
- exposing connection state

## Not responsible for

- WiFi connection
- Remote button control
- Position tracking
- LED GPIO handling
- Home Assistant entity behavior

## Public API

- `begin()`
- `update()`
- `isConnected()`
- `getMqtt()`
- `getDevice()`

## Design decision

The project uses the ArduinoHA library.  
The `MQTTManager` wraps `HADevice` and `HAMqtt` so other components can reuse the same Home Assistant MQTT connection.

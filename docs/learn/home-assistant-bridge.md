# HomeAssistant Bridge

## Responsibility

The `HomeAssistantBridge` exposes the awning as a Home Assistant cover entity.

It translates Home Assistant commands into actions on the `RemoteController`.

## Not responsible for

- WiFi connection
- MQTT connection
- GPIO details
- Position calculation logic
- RF transmission

## Flow

Home Assistant → MQTT → HomeAssistantBridge → RemoteController → Optocoupler → HSF1

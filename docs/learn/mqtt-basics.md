# MQTT Basics

## What is MQTT?

MQTT is a lightweight messaging protocol. It is often used in smart home systems because small devices can send and receive messages without needing to expose their own web server.

## Broker

The broker is the central message server. In this project, Home Assistant / Mosquitto acts as the broker. The ESP32 connects to the broker and exchanges messages through it.

## Publish and Subscribe

MQTT uses topics. A device can publish a message to a topic:

```text
3t-hsf1-bridge/status
```

Another device can subscribe to that topic and receive updates.

## Why MQTT instead of HTTP?

With HTTP, Home Assistant would have to call the ESP32 directly.

With MQTT, both Home Assistant and the ESP32 only need to connect to the broker.

This is more reliable for small devices and works well with Home Assistant.

## Home Assistant and MQTT

Home Assistant can discover devices via MQTT.

The ArduinoHA library helps with this by publishing the required discovery information automatically.

## In this project

The MQTTManager owns the MQTT connection.

It is responsible for:

- connecting to the MQTT broker
- keeping the MQTT loop running
- exposing the connection state
- providing access to HAMqtt and HADevice

It is not responsible for:

- WiFi
- LEDs
- remote button control
- position tracking

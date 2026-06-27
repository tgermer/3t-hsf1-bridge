# Wiring

## ESP32

| GPIO   | Connected to    | Description            |
| ------ | --------------- | ---------------------- |
| GPIO16 | Optocoupler IN1 | OPEN button            |
| GPIO17 | Optocoupler IN2 | STOP / Favorite button |
| GPIO18 | Optocoupler IN3 | CLOSE button           |
| GPIO25 | Green LED       | WiFi status            |
| GPIO26 | Blue LED        | MQTT status            |
| GPIO27 | Yellow LED      | Send indicator         |
| GPIO14 | Red LED         | Error indicator        |

## Optocoupler

| Input | Output     |
| ----- | ---------- |
| IN1   | HSF1 OPEN  |
| IN2   | HSF1 STOP  |
| IN3   | HSF1 CLOSE |

## Power

| Signal     | Connected to           |
| ---------- | ---------------------- |
| ESP32 3.3V | LEDs                   |
| ESP32 GND  | LEDs + Optocoupler GND |

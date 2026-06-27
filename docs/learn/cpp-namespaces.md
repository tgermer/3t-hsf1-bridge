# C++ Namespaces

## What is a namespace?

A namespace groups related identifiers together. Instead of writing:

```cpp
MQTT_HOST
```

we write:

```cpp
Config::MQTT::Host
```

Think of it like folders:

```text
Config
└── MQTT
    └── Host
```

The `::` operator means "inside".

## Why do we use namespaces?

- Better readability
- Avoid global constants
- Better project organization
- Easier maintenance

## 3. README ergänzen

Unter "Architecture" würde ich noch ergänzen:

```md
- Configuration (`Config.h`)
- Pin assignments (`Pins.h`)

Damit sieht jeder sofort, wo welche Art von Information zu finden ist.
```

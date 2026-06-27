# Development Philosophy

## Purpose

This project is more than a working ESP32 firmware.

It is intended to become a well-documented reference project for building maintainable firmware, learning modern software engineering practices and documenting architectural decisions.

## Principles

### Understand before implementing

Every feature should be understood before it is implemented.

Questions like:

- Why do we need this?
- What problem does it solve?
- Are there alternatives?

should be answered before writing code.

---

### One responsibility per class

Each class should have a single, clearly defined responsibility.

Examples:

- `WiFiManager` manages WiFi.
- `MQTTManager` manages MQTT.
- `RemoteController` simulates button presses.
- `PositionTracker` estimates the awning position.

---

### Small changes

Large changes should be split into small, understandable steps.

Every meaningful step should:

- compile
- be testable
- be committed separately

---

### Documentation is part of the project

Documentation is treated like source code.

Whenever architecture changes, the documentation should be updated as well.

---

### Learn while building

Whenever a new concept is introduced, it should be documented inside `docs/learn`.

The goal is not only to build the project, but also to understand it.

---

### Build often

Compile the project after small changes.

Small build errors are much easier to fix than large ones.

---

### Refactor continuously

Code should become cleaner over time.

Refactoring should improve readability and maintainability without changing behaviour.

---

### GitHub is part of the development process

Features and refactorings start with an Issue.

Commits should reference Issues.

Projects, Milestones and Labels are used to organize development.

---

### Think long-term

Always prefer code that is easier to understand in one year over code that is slightly shorter today.

Readability is more important than cleverness.

---

## Goal

The final repository should not only provide a working Home Assistant bridge.

It should also demonstrate professional software development practices and serve as a learning resource for future projects.

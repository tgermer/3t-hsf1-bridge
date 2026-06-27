# Soldering Basics

## Why this document?

This project is our first hardware project involving soldering.

The goal is not only to build a working ESP32 bridge, but also to learn good soldering techniques and document the experience for future projects.

---

## Equipment

- Temperature-controlled soldering iron
- Solder wire
- Side cutters
- Tweezers
- Breadboard
- Perfboard
- Multimeter
- Optocoupler module
- ESP32

---

## Golden Rule

**Heat the components, not the solder.**

The soldering iron should heat both the pad and the component lead.

Only then should solder be applied.

Correct order:

1. Place the soldering iron so it touches both the pad and the lead.
2. Wait 1–2 seconds.
3. Feed solder into the joint (not onto the soldering iron).
4. Remove the solder.
5. Remove the soldering iron.

---

## Characteristics of a good solder joint

A good solder joint should:

- be shiny
- have a smooth surface
- form a small cone around the lead
- completely cover the pad
- not move while cooling

---

## Common mistakes

### Cold solder joint

Cause:

- Components were not heated sufficiently.

Result:

- Dull surface
- Poor electrical contact

---

### Too much solder

Cause:

- Applying too much solder.

Result:

- Large blobs
- Risk of solder bridges

---

### Too little solder

Cause:

- Removing the solder too early.

Result:

- Weak mechanical connection

---

### Solder bridge

Cause:

- Too much solder between adjacent pads.

Result:

- Short circuit

Can often be removed using:

- solder wick
- desoldering pump

---

## Cleaning the tip

The soldering iron tip should always be clean.

Recommended:

- Brass wool
- Damp sponge

Before putting the soldering iron back into its holder:

- clean the tip
- apply a small amount of fresh solder

This protects the tip from oxidation.

---

## Practice before soldering expensive components

Recommended exercises:

- Create individual solder joints on a perfboard.
- Solder wires into pads.
- Solder resistors.
- Solder LEDs.
- Intentionally create and remove solder bridges.

Only after these exercises should expensive components such as the HSF1 remote be modified.

---

## Temperature

Typical temperatures:

| Material         | Temperature |
| ---------------- | ----------: |
| Leaded solder    |  320–350 °C |
| Lead-free solder |  350–380 °C |

Always use the lowest temperature that produces clean solder joints.

---

## Lessons learned

This section documents our own experience during this project.

(To be completed during hardware assembly.)

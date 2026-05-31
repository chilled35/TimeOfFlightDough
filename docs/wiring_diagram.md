# Wiring Diagram

## XIAO ESP32-S3 → VL53L5CX

### I²C Connection (400 kHz)

```
 XIAO ESP32-S3          VL53L5CX Breakout
 ┌───────────────┐          ┌────────────┐
 │  GPIO5 / D4  ├────────▶ SDA         │
 │  GPIO6 / D5  ├────────▶ SCL         │
 │  3V3         ├────────▶ VIN / 3V3   │
 │  GND         ├────────▶ GND         │
 └───────────────┘          └────────────┘
```

> **Note:** No additional pull-up resistors are needed — most VL53L5CX breakout boards include 4.7 kΩ pull-ups on SDA/SCL.

### VL53L5CX Breakout Board Power Requirements

The VL53L5CX sensor core runs at **1.8 V**, but virtually all commercial breakout boards include an onboard 1.8 V LDO and I²C level shifter. Connect **3.3 V** from the XIAO to the breakout’s VIN pin.

Popular compatible breakout boards:
- STMicroelectronics VL53L5CX-SATEL
- Adafruit VL53L5CX breakout (product #5699)
- Pololu VL53L5CX carrier

### I²C Address

Default I²C address: **0x29** (7-bit) = 0x52 (ST 8-bit notation).

The ESPHome YAML uses 7-bit addressing (`address: 0x29`).

---

## Physical Mounting

```
       Fridge top internal face
       ┌─────────────────────────────┐
       │       VL53L5CX (face down)     │
       │           ↓ FoV 45°            │
       │                               │
  ~20-30 cm                             │
       │                               │
       │   ┌─────────────────┐          │
       │   │  tray + cling film  │          │
       │   │    ╭───────╮        │          │
       │   │    │ dough │        │          │
       │   │    ╰───────╯        │          │
       │   └─────────────────┘          │
       └─────────────────────────────┘
```

**FoV footprint at 25 cm:** ~22 cm diameter. A 10 cm dough ball will occupy the central ~3–4 zones of the 8×8 grid.

**Tip:** Mount the sensor as close to the geometric centre of the tray as possible so the dough ball sits in zones 27–36 (rows 3–4, cols 3–4 in 0-indexed 8×8).

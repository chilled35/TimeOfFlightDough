# Calibration Guide

## Overview

Calibration records an **empty-baseline**: the distance from the sensor to the tray surface (with cling film in place) for every one of the 64 zones. Once stored, every subsequent reading publishes a `delta_mm` value for each zone:

```
delta_mm = baseline_mm - current_mm
```

**Positive delta = dough has risen** (dough is closer to the sensor than the empty tray was).

Calibration data is persisted to **ESP32 NVS** (flash preferences) and survives reboots.

---

## When to Calibrate

- First time you install the sensor.
- Any time you reposition the sensor or change the tray height.
- If you replace the cling film (negligible effect, but good practice).
- When the `binary_sensor.dough_monitor_calibration_valid` entity shows `off`.

---

## Procedure

### Step 1 — Prepare the Reference State

1. Remove any dough from the tray.
2. Place the empty tray in the fridge at its normal position.
3. Lay the cling film flat across the tray as you would normally.
4. Close the fridge door.
5. Wait ~30 seconds for the sensor to reach thermal equilibrium.

### Step 2 — Trigger Calibration

In Home Assistant:

1. Go to **Devices → Dough Monitor**.
2. Press the **“Capture Calibration Baseline”** button entity.
3. The ESP32 will collect and average **10 consecutive frames** (takes ~5 seconds at 2 fps).
4. `binary_sensor.dough_monitor_calibration_valid` will change to `on` when complete.

Alternatively, add a button card to your Lovelace dashboard:
```yaml
type: button
entity: button.dough_monitor_capture_calibration_baseline
name: Calibrate Sensor
icon: mdi:target
```

### Step 3 — Verify

1. Open the Dough Monitor dashboard.
2. With the empty tray in place, the 3D surface should appear **flat** (all deltas near 0).
3. The **Calibration Valid** badge should show green.
4. Place a dough ball in the tray; the surface should show a raised dome shape in the centre zones.

---

## Clearing Calibration

Press **“Clear Calibration”** button in HA. `delta_mm` will return zeros and `cal_valid` will be `false` in the JSON payload until a new baseline is captured.

---

## Technical Details

- The baseline is stored as 64 `uint16_t` values (128 bytes) in ESP32 NVS via ESPHome `Preferences`.
- Preference key is derived from `object_id_hash ^ 0xCAL1B00` to avoid collisions with other ESPHome preference keys.
- In **Averaged (Production)** mode the component averages `avg_window` frames before publishing. For calibration, 10 frames are always used regardless of this setting, to ensure a stable reference.
- Zone indexing is **row-major**, row 0 at the top of the sensor FoV (furthest from the PCB’s connector edge). Zone 27 = row 3, col 3 = geometric centre of the 8×8 grid.

---

## BME280 Correlation

Your existing Digital Dough Box BME280 sensors already expose temperature and humidity in HA. The HA package template sensors (`dough_max_rise`, `dough_proof_duration`) can be combined with BME280 data in Lovelace history cards to correlate dough rise rate against fridge temperature swings.

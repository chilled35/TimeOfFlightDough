# TimeOfFlightDough

An ESP32-S3-based dough proofing monitor using the STMicroelectronics **VL53L5CX** 8×8 Time-of-Flight sensor, integrated with **Home Assistant** via **ESPHome**. A real-time 3D height-map dashboard tracks dough-ball rise inside a converted fridge.

---

## Hardware

| Component | Part | Notes |
|---|---|---|
| Microcontroller | Seeed XIAO ESP32-S3 | 8 MB flash, 8 MB PSRAM |
| ToF sensor | STMicro VL53L5CX breakout | 8×8 zones, 45° FoV, up to 4 m |
| Fridge environment | BME280 ×2 | Already in Home Assistant as "Digital Dough Box" |
| Power | USB-C to XIAO | Or 3.3 V rail from fridge mod |

### Wiring (XIAO ESP32-S3 → VL53L5CX)

```
XIAO GPIO5 (D4)  ──── SDA
XIAO GPIO6 (D5)  ──── SCL
XIAO 3V3         ──── VDD / IOVDD
XIAO GND         ──── GND
```

Sensor is mounted on the top internal face of the fridge, pointing **straight down** toward the dough tray at ~20–30 cm range.

---

## Repository Layout

```
├── esphome/
│   ├── vl53l5cx_dough.yaml          # Main ESPHome configuration
│   ├── secrets.yaml.example         # Template — copy to secrets.yaml
│   └── components/
│       └── vl53l5cx/                # Custom ESPHome component
│           ├── __init__.py          # Component registration & schema
│           ├── sensor.py            # Per-zone & diagnostic sensor platform
│           ├── vl53l5cx_component.h # C++ class header
│           ├── vl53l5cx_component.cpp
│           └── driver/              # ST ULD driver files (see driver/DRIVER_README.md)
├── dashboard/
│   ├── dough_dashboard.html         # Standalone HTML — deploy to HA /local/
│   ├── js/
│   │   ├── visualizer.js            # Three.js 3D surface renderer
│   │   └── ha_websocket.js          # Home Assistant WebSocket client
│   └── css/
│       └── dashboard.css
├── ha_config/
│   └── packages/
│       └── dough_monitor.yaml       # HA helpers, automations, history config
└── docs/
    ├── wiring_diagram.md
    ├── installation.md
    └── calibration_guide.md
```

---

## Architecture Overview

```
 VL53L5CX
  (I²C)                     WiFi / Native API
    │                               │
    ▼                               ▼
 XIAO ESP32-S3  ──────────►  Home Assistant (HAOS / Proxmox)
  ESPHome                      │
                                ├── entity: sensor.dough_monitor_grid_data  (JSON text)
                                ├── entity: sensor.dough_monitor_centre_distance
                                ├── entity: button.capture_calibration_baseline
                                ├── entity: number.frame_rate_fps
                                └── entity: select.operating_mode
                                         │
                                         ▼
                               HA WebSocket API
                                         │
                                         ▼
                               /local/dough_dashboard/
                               dough_dashboard.html
                               (Three.js 3D surface)
```

---

## Data Flow

1. VL53L5CX → I²C → ESP32 (ESPHome custom component reads 64-zone distance + signal data)
2. Component serialises the 8×8 grid to a JSON string and publishes it as a **text sensor**
3. Home Assistant receives the text sensor state via the ESPHome native API
4. The HTML dashboard subscribes to state changes via the **HA WebSocket API**
5. Three.js renders the distance delta (vs calibration baseline) as a live 3D surface mesh

### JSON Payload Schema

```json
{
  "ts": 1717000000,
  "res": 8,
  "mode": "live",
  "distances_mm": [234, 235, 240, ...],
  "signal_kcps":  [512, 498, 501, ...],
  "nb_targets":   [1,   1,   1,   ...],
  "delta_mm":     [-12, -18, -45, ...],
  "cal_valid": true
}
```

`delta_mm` is `baseline_mm - current_mm` — **positive values indicate dough rise** above the empty-tray baseline.

---

## Operating Modes

| Mode | FPS | Purpose |
|---|---|---|
| Live (Testing) | 1–5 fps | Real-time 3D dashboard during setup/testing |
| Averaged (Production) | 0.016–0.1 fps (6–60 s interval) | Multi-frame averaging for stable rise tracking |

Both modes are selectable from a Home Assistant `select` entity without reflashing.

---

## Calibration

Calibration captures an **empty-tray + cling film** baseline. Once stored in ESP32 NVS (Preferences), every subsequent reading publishes `delta_mm` values relative to that baseline.

1. Remove dough ball, leave tray + cling film in place
2. Press **"Capture Calibration Baseline"** button in HA
3. ESP32 averages 10 consecutive frames and stores result to flash
4. `binary_sensor.calibration_valid` turns `on`

See [docs/calibration_guide.md](docs/calibration_guide.md) for full procedure.

---

## Installation

See [docs/installation.md](docs/installation.md) for step-by-step instructions covering:
- ESPHome CLI setup
- ST ULD driver integration
- Flashing the XIAO ESP32-S3
- HA integration
- Dashboard deployment

---

## Licence

MIT. The ST VL53L5CX ULD driver has its own licence — see `esphome/components/vl53l5cx/driver/`.

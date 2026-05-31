# Installation Guide

## Prerequisites

- Python 3.10+
- ESPHome CLI: `pip install esphome`
- Home Assistant OS (or Supervised) with file access to the `config/www/` folder
- USB-C cable for initial flash (OTA for subsequent updates)

---

## 1. Clone the Repository

```bash
git clone https://github.com/chilled35/TimeOfFlightDough.git
cd TimeOfFlightDough
```

---

## 2. Install the ST VL53L5CX ULD Driver

The ST driver files are **not bundled** due to licence restrictions.

1. Download the ULD from: https://www.st.com/en/embedded-software/stsw-img023.html
2. Extract the ZIP.
3. Copy the required C files (listed in `esphome/components/vl53l5cx/driver/DRIVER_README.md`) into `esphome/components/vl53l5cx/driver/`.
4. **Do not** overwrite `driver/platform.h` — our custom version must stay in place.

---

## 3. Configure Secrets

```bash
cp esphome/secrets.yaml.example esphome/secrets.yaml
```

Edit `esphome/secrets.yaml` and fill in:
- `wifi_ssid` / `wifi_password`
- `api_encryption_key` (generate with `esphome generate-api-key`)
- `ota_password`

`secrets.yaml` is in `.gitignore` and will never be committed.

---

## 4. Flash the ESP32-S3

First flash must be via USB:

```bash
cd esphome
esphome run vl53l5cx_dough.yaml
```

Subsequent updates use OTA (no USB needed):

```bash
esphome run vl53l5cx_dough.yaml --device dough-monitor.local
```

**Serial monitor** (useful during testing):
```bash
esphome logs vl53l5cx_dough.yaml
```

---

## 5. Add to Home Assistant

1. In HA: **Settings → Devices & Services → Add Integration → ESPHome**
2. Enter the device IP or hostname: `dough-monitor.local`
3. Enter the API encryption key from your `secrets.yaml`.
4. HA will discover all entities automatically.

---

## 6. Install the HA Package

1. Copy `ha_config/packages/dough_monitor.yaml` to your HA `config/packages/` folder (create the folder if it doesn’t exist).
2. Add to `config/configuration.yaml`:
   ```yaml
   homeassistant:
     packages:
       dough_monitor: !include packages/dough_monitor.yaml
   ```
3. **Settings → System → Restart** to apply.

---

## 7. Deploy the Dashboard

1. Copy the entire `dashboard/` folder to your HA config folder:
   ```
   config/www/dough_dashboard/
   ```
   The file structure should be:
   ```
   www/dough_dashboard/
   ├── dough_dashboard.html
   ├── js/
   │   ├── visualizer.js
   │   └── ha_websocket.js
   └── css/
       └── dashboard.css
   ```

2. Add an iframe panel to HA `configuration.yaml`:
   ```yaml
   panel_iframe:
     dough_monitor:
       title: "Dough Monitor"
       icon: mdi:grain
       url: "/local/dough_dashboard/dough_dashboard.html"
   ```
3. Restart HA.

4. Open the panel. Enter:
   - **HA URL:** `http://homeassistant.local:8123` (or your local IP)
   - **Token:** Create at HA → Profile → Long-Lived Access Tokens
   - **Entity ID:** `sensor.dough_monitor_grid_data` (default; adjust if device_name differs)

---

## 8. Verify

- HA → Developer Tools → States → search `dough_monitor` — you should see all entities.
- The `sensor.dough_monitor_grid_data` state should be a JSON string starting with `{"ts":`.
- The dashboard should connect and show a flat surface (no dough yet; calibrate first).

---

## Troubleshooting

| Symptom | Check |
|---|---|
| I²C scan shows no device | Wiring: SDA↔GPIO5, SCL↔GPIO6; power 3.3 V |
| `Sensor init failed` in logs | Confirm I²C address 0x29; try `scan: true` in i2c config |
| Dashboard says “auth invalid” | Token expired or wrong; re-generate in HA Profile |
| Grid data all zeros | ST ULD driver files missing or TODO stubs not yet replaced |
| OTA fails | Check device is on same VLAN; try by IP instead of hostname |

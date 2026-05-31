# ST VL53L5CX Ultra Lite Driver (ULD)

The ST ULD driver files are **not included** in this repository due to ST's licence terms. You must download them separately and place them here before building.

## Download

1. Go to: https://www.st.com/en/embedded-software/stsw-img023.html
2. Accept the licence and download the ZIP.
3. Extract the following files from `VL53L5CX_ULD_API/core/` into **this directory** (`esphome/components/vl53l5cx/driver/`):

```
vl53l5cx_api.h
vl53l5cx_api.c
vl53l5cx_buffers.h
vl53l5cx_plugin_detection_thresholds.h
vl53l5cx_plugin_detection_thresholds.c
vl53l5cx_plugin_motion_indicator.h
vl53l5cx_plugin_motion_indicator.c
vl53l5cx_plugin_xtalk.h
vl53l5cx_plugin_xtalk.c
platform.h          <- we provide a custom version of this (do NOT overwrite)
```

## Platform Abstraction

`platform.h` in this directory is our custom I²C bridge between the ST driver and ESPHome’s `i2c::I2CDevice`. Do **not** replace it with the version from the ST ZIP.

The ST ZIP contains a reference `platform.c` for STM32 — ours replaces it with `vl53l5cx_platform_esp.cpp` in the parent directory.

## Licence

The ST ULD is licenced under the ST SLA0081 licence. Review before redistribution.

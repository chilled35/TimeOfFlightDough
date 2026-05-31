# ST VL53L5CX Ultra Lite Driver (ULD)

The ST ULD driver files are **not included** in this repository due to ST's licence terms.
You must download them separately.

## Download

1. Go to: https://www.st.com/en/embedded-software/stsw-img023.html
2. Accept the licence and download the ZIP.
3. Extract the following files from `VL53L5CX_ULD_API/core/` and place them in the
   **component root** (`esphome/components/vl53l5cx/`) — NOT in this `driver/` subfolder:

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
```

## Platform Abstraction

`platform.h` in the component root is our custom I²C bridge. Do **not** replace it
with the version from the ST ZIP.

## Why not in this folder?

ESPHome's external component builder only copies files from the component root into
the PlatformIO build tree. Subdirectory contents are not included, so all driver
source files must sit alongside the component `.h/.cpp` files.

## Licence

The ST ULD is licenced under ST SLA0081. Review before redistribution.

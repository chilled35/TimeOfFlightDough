# ST VL53L8CX Ultra Lite Driver (ULD)

The ST ULD driver files are **not included** in this repository due to ST's licence terms.
You must download them separately.

## Download

1. Go to: https://www.st.com/en/embedded-software/stsw-img040.html
2. Accept the licence and download the ZIP (STSW-IMG040).
3. Extract the following files and place them in the
   **component root** (`esphome/components/vl53l5cx/`) — NOT in this `driver/` subfolder:

```
vl53l8cx_api.c
vl53l8cx_api.h
vl53l8cx_buffers.h
vl53l8cx_plugin_detection_thresholds.c
vl53l8cx_plugin_detection_thresholds.h
vl53l8cx_plugin_motion_indicator.c
vl53l8cx_plugin_motion_indicator.h
vl53l8cx_plugin_xtalk.c
vl53l8cx_plugin_xtalk.h
```

## Platform Abstraction

`platform.h` in the component root is our custom I²C bridge. Do **not** replace it
with the version from the ST ZIP.

## Required patch — missing semicolon in vl53l8cx_api.c

The released STSW-IMG040 package has a bug in `vl53l8cx_api.c`: a missing semicolon
on the `goto exit` statement inside `vl53l8cx_init`. Without this fix the file will
not compile.

Find the line (around line 368) that reads:

```c
		goto exit
```

and add the missing semicolon:

```c
		goto exit;
```

You can do it with sed (run from the component root):

```bash
# The line has a leading tab, so match the whole line explicitly:
sed -i '' '/^\t\tgoto exit$/ s/$/;/' vl53l8cx_api.c
```

Or just open the file in a text editor and add the `;` manually.

## Why not in this folder?

ESPHome's external component builder only copies files from the component root into
the PlatformIO build tree. Subdirectory contents are not included, so all driver
source files must sit alongside the component `.h/.cpp` files.

## Licence

The ST ULD is licenced under ST SLA0081. Review before redistribution.

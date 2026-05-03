# Product App RGBWW - Integration Notes

This file documents the bring-up and integration work completed so far for the GUITION ESP32-S3-4848S040 target.

## Current Status

- Boot is stable on ESP32-S3 with OPI PSRAM enabled.
- RGB panel init no longer crashes in ESP-IDF cleanup path.
- LVGL v8.4.0 is integrated and running through custom adapters.
- EEZ-generated UI from sibling repo `rgbww` is wired into app startup.
- Color rendering issue (segmented colorwheel) is fixed.

## Hardware Target

- SoC: ESP32-S3
- Panel: ST7701S over 3-wire SPI init + 16-bit RGB interface
- Resolution: 480x480
- Touch: GT911 (I2C)
- PSRAM: 8MB OPI
- Backlight: GPIO38 (active high)

## Key Fixes Applied

### 1) ESP-IDF RGB panel crash fix (LoadProhibited)

Issue:
- Null pointer dereference in `lcd_rgb_panel_destory()` when framebuffer allocation failed before HAL init.

Fix:
- Added null guard around `lcd_ll_enable_clock(rgb_panel->hal.dev, false)`.

Patch file:
- `patches/esp-idf-lcd_rgb_panel_rgb-null-hal-dev.patch`

### 2) Sming SDK component linkage

Issue:
- `esp_lcd` and `esp_psram` were not guaranteed in shared SDK component build.

Fix:
- Added both components to Sming SDK component list.

Patch file:
- `patches/sming-sdk-add-esp_lcd-esp_psram-components.patch`

### 3) LVGL runtime watchdog + flush synchronization

Issues:
- Task watchdog resets due to handler scheduling under unicore settings.
- Segmented widget rendering when LVGL draw buffer became reusable before transfer completion.

Fixes:
- Handler task adapted for unicore-safe behavior and non-zero delay ticks.
- Flush callback now waits for `display_driver_wait_flush_complete()` before `lv_disp_flush_ready()`.

### 4) Panel color format alignment

Issue:
- Basic colors looked mostly correct, but colorwheel gradient appeared sectorized/broken.

Fix:
- Aligned panel device `bits_per_pixel` to 18 (RGB666) while keeping 16-bit RGB bus transfer path.
- This matches the reference ESP32_Display_Panel configuration pattern for this board.

## Integration Architecture

### App level

- `app/application.cpp`
  - Turns backlight on (GPIO38)
  - Calls `esp_lcd_adapter_init(480, 480)`
  - Calls `ui_init()` from EEZ UI component

### Adapter chain

- `esp_lcd_adapter`
  - Creates and initializes board display driver
  - Starts LVGL runtime adapter

- `lvgl_runtime_adapter`
  - Owns LVGL display/input runtime
  - Allocates PSRAM draw buffers
  - Runs LVGL handler task
  - Flushes through abstract `display_driver_t`

- `display_driver_guition_4848s040`
  - ST7701S panel bring-up and RGB timing config
  - Implements `display_driver_t` contract

- `rgbww`
  - EEZ-generated UI sources (`src/ui/*`)

## Component Wiring

Project component dependencies now include:
- `esp_lcd_adapter`
- `rgbww`

Transitive dependencies include:
- `lvgl_runtime_adapter`
- `display_driver_guition_4848s040`
- `lvgl`

## LVGL Configuration

- `libs/lv_conf.h`
  - `LV_COLOR_DEPTH = 16`
  - `LV_COLOR_16_SWAP = 0`
  - `LV_TICK_CUSTOM = 1` via `esp_timer_get_time()`
  - Widgets/features enabled for current UI

## Build Notes

Use normal build command:

```bash
cd /home/pjakobs/devel/product_app_rgbww
make
```

Do not rely on truncating pipelines like `make | grep ... | head ...` for final build success checks. If output consumers terminate early, build subprocesses can be interrupted and leave partial artifacts.

## Relevant Paths

- App entry: `app/application.cpp`
- Project config: `component.mk`
- LVGL config: `libs/lv_conf.h`
- LVGL runtime: `../lvgl_runtime_adapter/src/lvgl_runtime_adapter.c`
- LCD adapter: `../esp_lcd_adapter/src/esp_lcd_adapter.c`
- Board driver: `../display_driver_guition_4848s040/src/display_driver_guition_4848s040.c`
- EEZ UI: `../rgbww/src/ui/`
- Patches: `patches/`

## Next Suggested Work

- Add GT911 touch callback wiring into `lvgl_runtime_adapter_set_touch_cb()`.
- Replace direct GPIO backlight toggle with dedicated backlight driver API.
- Add a minimal smoke test screen and boot self-check logs.
- Optionally split this README into user-facing and developer-facing docs.

## Sming component: abstract display driver interface + high-level adapter
## Provides display_driver_interface.h and esp_lcd_adapter.h.

COMPONENT_SRCDIRS  := src
COMPONENT_INCDIRS  := include

COMPONENT_DEPENDS  := display_driver_guition_4848s040 lvgl_runtime_adapter

## Only meaningful on ESP32 targets (ESP-IDF is required)
COMPONENT_SOC      := esp32*

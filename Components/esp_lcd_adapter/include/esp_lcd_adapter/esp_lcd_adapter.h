#pragma once

/**
 * esp_lcd_adapter.h
 *
 * High-level initialisation API.
 * Call esp_lcd_adapter_init() once from your application; it creates the board
 * display driver, initialises it, and starts LVGL via lvgl_runtime_adapter.
 */

#include <stdint.h>
#include "esp_err.h"
#include "esp_lcd_adapter/display_driver_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise the display driver and LVGL.
 *
 * @param width   Panel width in pixels (480 for GUITION-4848S040).
 * @param height  Panel height in pixels (480 for GUITION-4848S040).
 * @return ESP_OK on success.
 */
esp_err_t esp_lcd_adapter_init(uint16_t width, uint16_t height);

/**
 * Return the underlying display_driver_t (valid after esp_lcd_adapter_init).
 */
display_driver_t *esp_lcd_adapter_get_driver(void);

#ifdef __cplusplus
}
#endif

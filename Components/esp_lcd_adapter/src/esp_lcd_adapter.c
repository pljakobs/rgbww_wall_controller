/**
 * esp_lcd_adapter.c
 *
 * Bridge between LVGL runtime and esp_lcd drivers.
 * Creates a display_driver_guition_4848s040, initialises it, then hands it to
 * lvgl_runtime_adapter which wires LVGL flush callbacks and starts the handler task.
 */

#include "esp_lcd_adapter/display_driver_interface.h"
#include "display_driver_guition_4848s040/display_driver_guition_4848s040.h"
#include "lvgl_runtime_adapter/lvgl_runtime_adapter.h"

#include "esp_log.h"

static const char *TAG = "esp_lcd_adapter";

static display_driver_t *s_driver;

esp_err_t esp_lcd_adapter_init(uint16_t width, uint16_t height)
{
    display_driver_guition_4848s040_config_t cfg = {
        .base = {
            .width  = width,
            .height = height,
        },
        .pclk_frequency_hz = 26000000U,
        .pclk_inverted     = false,
        .dma_buffer_size   = 10,
    };

    esp_err_t ret = display_driver_guition_4848s040_create(&cfg, &s_driver);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "display driver create failed: %d", ret);
        return ret;
    }

    display_driver_config_t base_cfg = { .width = width, .height = height };
    ret = display_driver_init(s_driver, &base_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "display driver init failed: %d", ret);
        display_driver_guition_4848s040_destroy(s_driver);
        s_driver = NULL;
        return ret;
    }

    ret = lvgl_runtime_adapter_init(s_driver);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "lvgl_runtime_adapter_init failed: %d", ret);
        return ret;
    }

    ESP_LOGI(TAG, "display + LVGL ready (%"PRIu16"x%"PRIu16")", width, height);
    return ESP_OK;
}

display_driver_t *esp_lcd_adapter_get_driver(void)
{
    return s_driver;
}


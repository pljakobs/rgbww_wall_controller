#include <SmingCore.h>

extern "C" {
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_lcd_adapter/esp_lcd_adapter.h"
#include "lvgl_runtime_adapter/lvgl_runtime_adapter.h"
#include "ui/ui.h"
}

/* Backlight is on GPIO 38, active-high PWM/GPIO on the GUITION-4848S040. */
static constexpr gpio_num_t BACKLIGHT_GPIO = GPIO_NUM_38;

static void backlight_on()
{
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = (1ULL << BACKLIGHT_GPIO);
    cfg.mode        = GPIO_MODE_OUTPUT;
    cfg.pull_up_en  = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type   = GPIO_INTR_DISABLE;
    gpio_config(&cfg);
    gpio_set_level(BACKLIGHT_GPIO, 1);
}

static const char* TAG = "APP";

static constexpr uint16_t WIDTH  = 480;
static constexpr uint16_t HEIGHT = 480;

void init()
{
    ESP_LOGI(TAG, "=== init() start ===");

    backlight_on();

    if (esp_lcd_adapter_init(WIDTH, HEIGHT) != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_adapter_init FAILED");
        return;
    }
    ESP_LOGI(TAG, "Display + LVGL ready");

    lvgl_runtime_adapter_lock();
    ui_init();
    lvgl_runtime_adapter_unlock();

    ESP_LOGI(TAG, "=== init() complete ===");

    vTaskSuspend(NULL);
}

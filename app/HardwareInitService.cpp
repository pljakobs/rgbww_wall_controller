#include "HardwareInitService.h"

extern "C" {
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_lcd_adapter/esp_lcd_adapter.h"
#include "lvgl_runtime_adapter/lvgl_runtime_adapter.h"
#include "touch_driver_gt911/touch_driver_gt911.h"
}

static const char* HW_TAG = "HwInit";

static constexpr gpio_num_t BACKLIGHT_GPIO  = GPIO_NUM_38;
static constexpr uint16_t   DISPLAY_WIDTH   = 480;
static constexpr uint16_t   DISPLAY_HEIGHT  = 480;
static constexpr gpio_num_t TOUCH_SDA_GPIO  = GPIO_NUM_19;
static constexpr gpio_num_t TOUCH_SCL_GPIO  = GPIO_NUM_45;

static void backlight_on()
{
    gpio_config_t cfg = {};
    cfg.pin_bit_mask   = (1ULL << BACKLIGHT_GPIO);
    cfg.mode           = GPIO_MODE_OUTPUT;
    cfg.pull_up_en     = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en   = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type      = GPIO_INTR_DISABLE;
    gpio_config(&cfg);
    gpio_set_level(BACKLIGHT_GPIO, 1);
}

static bool touch_read_cb(int16_t* x, int16_t* y)
{
    return touch_driver_gt911_read(x, y);
}

namespace lightinator {

HardwareCapabilities HardwareInitService::init()
{
    HardwareCapabilities caps;

    backlight_on();

    if (esp_lcd_adapter_init(DISPLAY_WIDTH, DISPLAY_HEIGHT) != ESP_OK) {
        ESP_LOGE(HW_TAG, "esp_lcd_adapter_init failed");
        return caps; // displayOk stays false
    }
    caps.displayOk = true;

    const touch_driver_gt911_config_t touch_cfg = {
        .i2c_port    = 0,
        .sda_gpio    = TOUCH_SDA_GPIO,
        .scl_gpio    = TOUCH_SCL_GPIO,
        .int_gpio    = GPIO_NUM_NC,
        .rst_gpio    = GPIO_NUM_NC,
        .width       = DISPLAY_WIDTH,
        .height      = DISPLAY_HEIGHT,
        .mirror_x    = false,
        .mirror_y    = false,
        .swap_xy     = false,
        .i2c_clock_hz = 400000,
    };

    if (touch_driver_gt911_init(&touch_cfg) == ESP_OK) {
        lvgl_runtime_adapter_set_touch_cb(touch_read_cb);
        ESP_LOGI(HW_TAG, "GT911 touch ready");
        caps.touchOk = true;
    } else {
        ESP_LOGW(HW_TAG, "GT911 touch init failed; continuing without touch");
    }

    return caps;
}

} // namespace lightinator

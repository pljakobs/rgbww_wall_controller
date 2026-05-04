#include <SmingCore.h>

extern "C" {
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_lcd_adapter/esp_lcd_adapter.h"
#include "lvgl_runtime_adapter/lvgl_runtime_adapter.h"
#include "touch_driver_gt911/touch_driver_gt911.h"
}

#include "ui/AppUi.h"
#include "ui/WifiConfigFlow.h"
#include "networking.h"
#include "NetworkUiBinder.h"
#include <app-config.h>
#include <vector>

static const char* TAG = "APP";
static constexpr gpio_num_t BACKLIGHT_GPIO = GPIO_NUM_38;
static constexpr uint16_t WIDTH = 480;
static constexpr uint16_t HEIGHT = 480;
static constexpr gpio_num_t TOUCH_SDA_GPIO = GPIO_NUM_19;
static constexpr gpio_num_t TOUCH_SCL_GPIO = GPIO_NUM_45;

static lightinator::ui::AppUi s_ui;
static SimpleTimer s_ui_timer;
static std::unique_ptr<AppConfig> s_cfg;
static std::unique_ptr<AppWIFI> s_wifi;
static std::unique_ptr<lightinator::ui::WifiConfigFlow> s_wifi_flow;
static std::unique_ptr<lightinator::NetworkUiBinder> s_net_ui_binder;

static void backlight_on()
{
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = (1ULL << BACKLIGHT_GPIO);
    cfg.mode = GPIO_MODE_OUTPUT;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&cfg);
    gpio_set_level(BACKLIGHT_GPIO, 1);
}

static bool app_touch_read(int16_t* x, int16_t* y)
{
    return touch_driver_gt911_read(x, y);
}

static void ui_tick()
{
    lvgl_runtime_adapter_lock();
    s_ui.tickAnimation();
    lvgl_runtime_adapter_unlock();
}

void init()
{
    ESP_LOGI(TAG, "=== init() start ===");

    s_cfg = std::make_unique<AppConfig>("app-config");
    s_wifi = std::make_unique<AppWIFI>(*s_cfg);
    s_wifi_flow = std::make_unique<lightinator::ui::WifiConfigFlow>(s_ui, *s_wifi);
    s_net_ui_binder = std::make_unique<lightinator::NetworkUiBinder>(s_ui, *s_wifi, s_wifi_flow.get());

    backlight_on();

    if (esp_lcd_adapter_init(WIDTH, HEIGHT) != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_adapter_init failed");
        return;
    }

    const touch_driver_gt911_config_t touch_cfg = {
        .i2c_port = 0,
        .sda_gpio = TOUCH_SDA_GPIO,
        .scl_gpio = TOUCH_SCL_GPIO,
        .int_gpio = GPIO_NUM_NC,
        .rst_gpio = GPIO_NUM_NC,
        .width = WIDTH,
        .height = HEIGHT,
        .mirror_x = false,
        .mirror_y = false,
        .swap_xy = false,
        .i2c_clock_hz = 400000,
    };

    if (touch_driver_gt911_init(&touch_cfg) == ESP_OK) {
        lvgl_runtime_adapter_set_touch_cb(app_touch_read);
        ESP_LOGI(TAG, "GT911 touch ready");
    } else {
        ESP_LOGW(TAG, "GT911 touch init failed; continuing without touch");
    }

    lvgl_runtime_adapter_lock();
    const bool ui_ready = s_ui.init();
    lvgl_runtime_adapter_unlock();
    if (!ui_ready) {
        ESP_LOGE(TAG, "handwritten UI init failed");
        return;
    }

    // Register WiFi->UI callbacks only after LVGL runtime and UI are initialized.
    s_net_ui_binder->bind();

    s_wifi->init();
    lvgl_runtime_adapter_lock();
    s_net_ui_binder->syncState();
    lvgl_runtime_adapter_unlock();

    if (s_wifi_flow) {
        s_wifi_flow->startIfNeeded();
    }

    s_ui_timer.initializeMs(180, ui_tick).start();
    ESP_LOGI(TAG, "=== init() complete ===");
}

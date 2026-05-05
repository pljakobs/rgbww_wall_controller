#include "HardwareInitService.h"

extern "C" {
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_lcd_adapter/esp_lcd_adapter.h"
#include "lvgl_runtime_adapter/lvgl_runtime_adapter.h"
#include "touch_driver_gt911/touch_driver_gt911.h"
}

static const char* HW_TAG = "HwInit";

static constexpr gpio_num_t BACKLIGHT_GPIO  = GPIO_NUM_38;
static constexpr ledc_mode_t BACKLIGHT_LEDC_MODE = LEDC_LOW_SPEED_MODE;
static constexpr ledc_timer_t BACKLIGHT_LEDC_TIMER = LEDC_TIMER_0;
static constexpr ledc_channel_t BACKLIGHT_LEDC_CHANNEL = LEDC_CHANNEL_0;
static constexpr ledc_timer_bit_t BACKLIGHT_LEDC_RESOLUTION = LEDC_TIMER_13_BIT;
static constexpr uint32_t BACKLIGHT_LEDC_MAX_DUTY = (1U << BACKLIGHT_LEDC_RESOLUTION) - 1U;
static constexpr uint16_t   DISPLAY_WIDTH   = 480;
static constexpr uint16_t   DISPLAY_HEIGHT  = 480;
static constexpr gpio_num_t TOUCH_SDA_GPIO  = GPIO_NUM_19;
static constexpr gpio_num_t TOUCH_SCL_GPIO  = GPIO_NUM_45;

static lightinator::HardwareInitService* s_activeHardwareService = nullptr;

static bool touch_read_cb(int16_t* x, int16_t* y)
{
    return (s_activeHardwareService != nullptr) ? s_activeHardwareService->readTouch(x, y) : false;
}

namespace lightinator {

namespace {

uint32_t brightnessToDuty(int brightnessPercent)
{
    if (brightnessPercent <= 0) {
        return 0;
    }
    if (brightnessPercent >= 100) {
        return BACKLIGHT_LEDC_MAX_DUTY;
    }
    return static_cast<uint32_t>((static_cast<uint64_t>(brightnessPercent) * BACKLIGHT_LEDC_MAX_DUTY) / 100U);
}

touch_driver_gt911_calibration_t toDriverCalibration(const HardwareTouchCalibration& calibration)
{
    touch_driver_gt911_calibration_t driverCalibration = {};
    driverCalibration.enabled = calibration.enabled;
    driverCalibration.a = calibration.a;
    driverCalibration.b = calibration.b;
    driverCalibration.c = calibration.c;
    driverCalibration.d = calibration.d;
    driverCalibration.e = calibration.e;
    driverCalibration.f = calibration.f;
    return driverCalibration;
}

} // namespace

HardwareInitService::~HardwareInitService()
{
    shutdown();
}

HardwareCapabilities HardwareInitService::init(const HardwareInitOptions& options)
{
    shutdown();

    HardwareCapabilities caps;

    brightnessPercent_ = options.brightnessPercent;
    backlightTimeoutSeconds_ = options.backlightTimeoutSeconds;
    backlightAwake_ = true;
    swallowWakeTouch_ = false;
    lastTouchUs_ = esp_timer_get_time();

    const ledc_timer_config_t timerConfig = {
        .speed_mode = BACKLIGHT_LEDC_MODE,
        .duty_resolution = BACKLIGHT_LEDC_RESOLUTION,
        .timer_num = BACKLIGHT_LEDC_TIMER,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false,
    };
    if (ledc_timer_config(&timerConfig) != ESP_OK) {
        ESP_LOGE(HW_TAG, "backlight timer init failed");
        return caps;
    }

    const ledc_channel_config_t channelConfig = {
        .gpio_num = BACKLIGHT_GPIO,
        .speed_mode = BACKLIGHT_LEDC_MODE,
        .channel = BACKLIGHT_LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = BACKLIGHT_LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    if (ledc_channel_config(&channelConfig) != ESP_OK) {
        ESP_LOGE(HW_TAG, "backlight channel init failed");
        return caps;
    }
    applyBacklightOutput(true);

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
        .stable_press_ms = options.touchStablePressMs,
        .calibration = toDriverCalibration(options.touchCalibration),
    };

    if (touch_driver_gt911_init(&touch_cfg) == ESP_OK) {
        touchInitialized_ = true;
        s_activeHardwareService = this;
        lvgl_runtime_adapter_set_touch_cb(touch_read_cb);
        ESP_LOGI(HW_TAG, "GT911 touch ready");
        caps.touchOk = true;
    } else {
        ESP_LOGW(HW_TAG, "GT911 touch init failed; continuing without touch");
    }

    return caps;
}

void HardwareInitService::shutdown()
{
    if (s_activeHardwareService == this) {
        s_activeHardwareService = nullptr;
        lvgl_runtime_adapter_set_touch_cb(nullptr);
    }

    if (touchInitialized_) {
        touch_driver_gt911_deinit();
        touchInitialized_ = false;
    }
}

void HardwareInitService::setBacklightBrightness(int brightnessPercent)
{
    if (brightnessPercent < 0) {
        brightnessPercent = 0;
    } else if (brightnessPercent > 100) {
        brightnessPercent = 100;
    }
    brightnessPercent_ = brightnessPercent;
    applyBacklightOutput(backlightAwake_);
}

void HardwareInitService::setBacklightTimeoutSeconds(int timeoutSeconds)
{
    backlightTimeoutSeconds_ = timeoutSeconds;
    lastTouchUs_ = esp_timer_get_time();
    if (!backlightAwake_) {
        backlightAwake_ = true;
        swallowWakeTouch_ = false;
        applyBacklightOutput(true);
    }
}

void HardwareInitService::applyTouchCalibration(const HardwareTouchCalibration& calibration)
{
    const touch_driver_gt911_calibration_t driverCalibration = toDriverCalibration(calibration);
    touch_driver_gt911_set_calibration(&driverCalibration);
}

bool HardwareInitService::readTouch(int16_t* x, int16_t* y)
{
    const bool pressed = touch_driver_gt911_read(x, y);
    const int64_t nowUs = esp_timer_get_time();

    if (swallowWakeTouch_) {
        if (!pressed) {
            swallowWakeTouch_ = false;
        }
        return false;
    }

    if (backlightTimeoutSeconds_ >= 0 && backlightAwake_ && !pressed && lastTouchUs_ > 0) {
        const int64_t timeoutUs = static_cast<int64_t>(backlightTimeoutSeconds_) * 1000000LL;
        if ((nowUs - lastTouchUs_) >= timeoutUs) {
            backlightAwake_ = false;
            applyBacklightOutput(false);
        }
    }

    if (!backlightAwake_) {
        if (pressed) {
            backlightAwake_ = true;
            swallowWakeTouch_ = true;
            lastTouchUs_ = nowUs;
            applyBacklightOutput(true);
        }
        return false;
    }

    if (pressed) {
        lastTouchUs_ = nowUs;
        return true;
    }

    return false;
}

void HardwareInitService::applyBacklightOutput(bool enabled)
{
    const uint32_t duty = enabled ? brightnessToDuty(brightnessPercent_) : 0;
    ledc_set_duty(BACKLIGHT_LEDC_MODE, BACKLIGHT_LEDC_CHANNEL, duty);
    ledc_update_duty(BACKLIGHT_LEDC_MODE, BACKLIGHT_LEDC_CHANNEL);
}

} // namespace lightinator

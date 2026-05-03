#include <SmingCore.h>

/*
 * Application working model
 * -------------------------
 * This file is the glue layer between four subsystems:
 *
 * 1. Sming application startup
 *    `init()` is called once at boot and is our only top-level entrypoint.
 *
 * 2. The display + LVGL runtime adapters
 *    `esp_lcd_adapter_init()` brings up the board display driver and then
 *    starts LVGL through `lvgl_runtime_adapter`.
 *
 * 3. Generated EEZ/LVGL UI wrappers
 *    `s_ui` is a generated object graph that gives us typed access to the
 *    widgets exported by the EEZ project, such as buttons and LEDs.
 *
 * 4. Application behavior
 *    This file adds touch input, button event handling, and a timer-driven LED
 *    animation on top of the generated UI.
 *
 * The important ownership rule is:
 * - LVGL owns the underlying `lv_obj_t*` widgets.
 * - The generated connector stores typed references/wrappers to those widgets.
 * - Application code talks to widgets only through the generated connector.
 *
 * The important threading rule is:
 * - LVGL itself is serviced by the runtime adapter task.
 * - Any application code that touches LVGL objects outside of an LVGL event
 *   callback must hold the LVGL lock.
 * - Therefore timer-driven UI updates below use
 *   `lvgl_runtime_adapter_lock()/unlock()`.
 */

extern "C" {
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_lcd_adapter/esp_lcd_adapter.h"
#include "lvgl_runtime_adapter/lvgl_runtime_adapter.h"
#include "touch_driver_gt911/touch_driver_gt911.h"
#include "ui/ui.h"
}

#include "ui_codegen/generated/eez_ui_connector.hpp"

/* Backlight is on GPIO 38, active-high on the GUITION-4848S040 board. */
static constexpr gpio_num_t BACKLIGHT_GPIO = GPIO_NUM_38;

/*
 * The board ships with the TFT backlight disabled after reset.
 * Turning the display subsystem on without enabling the backlight can make the
 * system appear dead even though LVGL is already rendering correctly.
 */
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
static constexpr gpio_num_t TOUCH_SDA_GPIO = GPIO_NUM_19;
static constexpr gpio_num_t TOUCH_SCL_GPIO = GPIO_NUM_45;

/*
 * `s_ui` owns the generated connector/instantiator. Calling `s_ui.init()`
 * executes the generated `ui_init()` function and then binds the generated C++
 * wrappers to the actual LVGL objects created by the EEZ-generated C code.
 */
static product::ui::EezUiInstantiator s_ui;

/*
 * Sming timer driving the demo LED animation.
 *
 * This timer does not run LVGL itself. It merely schedules our application
 * callback, which then updates a subset of the LED widgets under the LVGL
 * lock. LVGL's own handler loop is managed separately by `lvgl_runtime_adapter`.
 */
static SimpleTimer s_led_chase_timer;

/*
 * Animation state:
 * - `s_led_chase_step` is the phase accumulator for the color animation.
 * - `s_led_brightness_initialized` avoids repeatedly writing LED brightness,
 *   because only hue needs to change continuously.
 */
static uint32_t s_led_chase_step = 0;
static bool s_led_brightness_initialized = false;

/*
 * LVGL asks the runtime adapter for touch state through a simple callback with
 * `(x, y) -> pressed` semantics. This adapter function keeps the application
 * side decoupled from LVGL internals and lets the GT911 driver expose a very
 * small polling API.
 */
static bool app_touch_read(int16_t* x, int16_t* y)
{
    return touch_driver_gt911_read(x, y);
}

/*
 * We use one shared LVGL callback for all six color buttons.
 * `AppButtonId` is passed as LVGL user data so the callback can determine which
 * logical action to perform without having to create six separate handlers.
 */
enum class AppButtonId {
    Blue,
    Red,
    Green,
    Teal,
    Yellow,
    Pink,
};

static const char* button_name(AppButtonId id)
{
    switch (id) {
    case AppButtonId::Blue: return "Blue";
    case AppButtonId::Red: return "Red";
    case AppButtonId::Green: return "Green";
    case AppButtonId::Teal: return "Teal";
    case AppButtonId::Yellow: return "Yellow";
    case AppButtonId::Pink: return "Pink";
    default: return "Unknown";
    }
}

static lv_color_t button_color(AppButtonId id)
{
    switch (id) {
    case AppButtonId::Blue: return lv_palette_main(LV_PALETTE_BLUE);
    case AppButtonId::Red: return lv_palette_main(LV_PALETTE_RED);
    case AppButtonId::Green: return lv_palette_main(LV_PALETTE_GREEN);
    case AppButtonId::Teal: return lv_palette_main(LV_PALETTE_TEAL);
    case AppButtonId::Yellow: return lv_palette_main(LV_PALETTE_YELLOW);
    case AppButtonId::Pink: return lv_palette_main(LV_PALETTE_PINK);
    default: return lv_color_white();
    }
}

/*
 * Convenience helper used by button events.
 *
 * The generated connector exposes LEDs by stable widget IDs, so the app can
 * enumerate the LED widgets explicitly and update them in one place.
 */
static void apply_color_to_all_leds(lv_color_t color)
{
    using WidgetId = product::ui::EezUiConnector::WidgetId;

    static constexpr WidgetId kLedIds[] = {
        WidgetId::LED1,
        WidgetId::LED2,
        WidgetId::LED3,
        WidgetId::LED4,
        WidgetId::LED5,
        WidgetId::LED6,
        WidgetId::LED7,
        WidgetId::LED8,
        WidgetId::LED9,
    };

    auto& ui = s_ui.ui();
    for (auto id : kLedIds) {
        auto* led = ui.asLed(id);
        if (!led || !led->isBound()) {
            continue;
        }
        led->setColor(color);
        led->setBrightness(255);
    }
}

/*
 * Shared LVGL event callback for the color buttons.
 *
 * This runs in LVGL's event context, so it can directly manipulate widgets.
 * Current behavior is intentionally simple for demonstration purposes:
 * - stop the autonomous animation
 * - paint all LEDs in the color associated with the clicked button
 * - log the interaction
 */
static void on_button_event(lv_event_t* event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    auto* id = static_cast<AppButtonId*>(lv_event_get_user_data(event));
    if (id == nullptr) {
        return;
    }

    s_led_chase_timer.stop();
    s_led_brightness_initialized = true;
    apply_color_to_all_leds(button_color(*id));
    ESP_LOGI(TAG, "%s button clicked", button_name(*id));
}

/*
 * Register application behavior against generated widgets.
 *
 * The generated wrappers already know how to bind to the concrete LVGL button
 * objects. We attach one callback per button and identify the source through
 * the stable `AppButtonId` values passed as user data.
 */
static void register_button_callbacks()
{
    auto& ui = s_ui.ui();

    static AppButtonId blueId = AppButtonId::Blue;
    static AppButtonId redId = AppButtonId::Red;
    static AppButtonId greenId = AppButtonId::Green;
    static AppButtonId tealId = AppButtonId::Teal;
    static AppButtonId yellowId = AppButtonId::Yellow;
    static AppButtonId pinkId = AppButtonId::Pink;

    ui.blue_button().addCallback(on_button_event, LV_EVENT_CLICKED, &blueId);
    ui.red_button().addCallback(on_button_event, LV_EVENT_CLICKED, &redId);
    ui.green_button().addCallback(on_button_event, LV_EVENT_CLICKED, &greenId);
    ui.teal_buton().addCallback(on_button_event, LV_EVENT_CLICKED, &tealId);
    ui.yellow_button().addCallback(on_button_event, LV_EVENT_CLICKED, &yellowId);
    ui.pink_button().addCallback(on_button_event, LV_EVENT_CLICKED, &pinkId);
}

/*
 * Periodic demo animation.
 *
 * Visual model:
 * - all 9 LEDs represent a hue wheel
 * - each LED is offset by 360/9 degrees
 * - only one third of the LEDs are updated per tick to keep CPU usage lower
 *
 * Concurrency model:
 * - the callback is triggered by Sming's timer system
 * - because it is not an LVGL event callback, it must acquire the LVGL lock
 *   before touching any widget
 */
static void led_chase_tick()
{
    using WidgetId = product::ui::EezUiConnector::WidgetId;

    static constexpr WidgetId kLedIds[] = {
        WidgetId::LED1,
        WidgetId::LED2,
        WidgetId::LED3,
        WidgetId::LED4,
        WidgetId::LED5,
        WidgetId::LED6,
        WidgetId::LED7,
        WidgetId::LED8,
        WidgetId::LED9,
    };

    /* Advance the whole wheel by 6 degrees per app tick. */
    static constexpr uint16_t kHueStepDegrees = 6;
    /* Neighboring LEDs are evenly distributed around the hue circle. */
    static constexpr uint16_t kPhaseOffsetDegrees = 360 / 9;
    static constexpr uint8_t kSaturation = 100;
    static constexpr uint8_t kValue = 100;
    static constexpr uint8_t kBrightness = 255;
    /* Update LEDs in 3 interleaved groups to reduce redraw load per tick. */
    static constexpr size_t kUpdateGroups = 3;

    lvgl_runtime_adapter_lock();

    auto& ui = s_ui.ui();
    const uint16_t baseHue = static_cast<uint16_t>((s_led_chase_step * kHueStepDegrees) % 360);

    /* Brightness is static for this demo, so initialize it once. */
    if (!s_led_brightness_initialized) {
        for (size_t i = 0; i < sizeof(kLedIds) / sizeof(kLedIds[0]); ++i) {
            auto* led = ui.asLed(kLedIds[i]);
            if (!led || !led->isBound()) {
                continue;
            }
            led->setBrightness(kBrightness);
        }
        s_led_brightness_initialized = true;
    }

    /* Only one group is recolored on each tick. */
    const size_t group = s_led_chase_step % kUpdateGroups;
    for (size_t i = group; i < sizeof(kLedIds) / sizeof(kLedIds[0]); i += kUpdateGroups) {
        auto* led = ui.asLed(kLedIds[i]);
        if (!led || !led->isBound()) {
            continue;
        }
        const uint16_t hue = static_cast<uint16_t>((baseHue + (i * kPhaseOffsetDegrees)) % 360);
        led->setColor(lv_color_hsv_to_rgb(hue, kSaturation, kValue));
    }

    lvgl_runtime_adapter_unlock();
    ++s_led_chase_step;
}

/*
 * Sming boot entrypoint.
 *
 * Startup sequence:
 * 1. Enable panel backlight so the screen is visible.
 * 2. Initialize the display driver and LVGL runtime.
 * 3. Initialize the GT911 touch controller and register the touch callback.
 * 4. Initialize the generated EEZ UI and bind wrappers to live LVGL objects.
 * 5. Attach application callbacks to buttons.
 * 6. Start the demo LED animation timer.
 *
 * At the end of `init()` the system is event-driven:
 * - LVGL handler task keeps widgets alive and dispatches touch/events.
 * - Sming timer updates the LEDs periodically.
 * - Button presses arrive as LVGL events and are routed through
 *   `on_button_event()`.
 */
void init()
{
    ESP_LOGI(TAG, "=== init() start ===");

    backlight_on();

    if (esp_lcd_adapter_init(WIDTH, HEIGHT) != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_adapter_init FAILED");
        return;
    }
    ESP_LOGI(TAG, "Display + LVGL ready");

    /*
     * Touch transform values describe how raw controller coordinates map onto
     * the display orientation currently used by the panel.
     */
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

    /*
     * UI creation and callback registration both touch LVGL objects, so they
     * must happen while holding the LVGL lock.
     */
    lvgl_runtime_adapter_lock();
    if (!s_ui.init()) {
        ESP_LOGE(TAG, "UI instantiator init failed");
        lvgl_runtime_adapter_unlock();
        return;
    }
    register_button_callbacks();
    lvgl_runtime_adapter_unlock();

    s_led_chase_timer.initializeMs(180, led_chase_tick).start();
    ESP_LOGI(TAG, "LED chase timer started");

    ESP_LOGI(TAG, "=== init() complete ===");
}

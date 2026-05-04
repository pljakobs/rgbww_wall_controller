#include <SmingCore.h>
#include <esp_log.h>

#include "ui/AppUi.h"
#include "ui/core/UiTheme.h"
#include "ui/WifiConfigFlow.h"
#include "networking.h"
#include "NetworkUiBinder.h"
#include "HardwareInitService.h"
#include "UiRuntimeService.h"
#include <Storage/SysMem.h>
#include <Storage/ProgMem.h>
#include <Storage/Debug.h>
#include <LittleFS.h>
#include <app-config.h>
#include <app-data.h>

static const char* TAG = "APP";

static lightinator::ui::AppUi               s_ui;
static lightinator::HardwareInitService     s_hw;
static lightinator::UiRuntimeService        s_ui_runtime;
static std::unique_ptr<AppConfig>           s_cfg;
static std::unique_ptr<AppData>             s_data;
static std::unique_ptr<AppWIFI>             s_wifi;
static std::unique_ptr<lightinator::ui::WifiConfigFlow> s_wifi_flow;
static std::unique_ptr<lightinator::NetworkUiBinder>    s_net_ui_binder;
bool fsMounted = false;

void init()
{
    ESP_LOGI(TAG, "=== init() start ===");

    auto part = Storage::findPartition(F("lfs"));
    if(part) {
        if(lfs_mount(part)) {
            fsMounted = true;
        } 
    }

    // Compose application modules.
    s_cfg       = std::make_unique<AppConfig>("app-config");
    s_data      = std::make_unique<AppData>("app-data");

    // Load active theme from ConfigDB, falling back to built-in Nordic Dark.
    {
        using namespace lightinator::ui::core;
        UiTheme theme = nordicDarkTheme();
        String activeId = s_cfg->getActiveTheme();
        for (auto t : s_data->themes) {
            if (t.getId() == activeId) {
                theme.id           = t.getId();
                theme.name         = t.getName();
                theme.mode         = (t.getMode() == 0) ? "light" : "dark";
                theme.headerHeight = static_cast<lv_coord_t>(t.getHeaderHeight());
                theme.colors.headerBg  = colorFromHexString(t.colors.getHeaderBg(),  theme.colors.headerBg);
                theme.colors.headerFg  = colorFromHexString(t.colors.getHeaderFg(),  theme.colors.headerFg);
                theme.colors.contentBg = colorFromHexString(t.colors.getContentBg(), theme.colors.contentBg);
                theme.colors.contentFg = colorFromHexString(t.colors.getContentFg(), theme.colors.contentFg);
                theme.colors.buttonBg  = colorFromHexString(t.colors.getButtonBg(),  theme.colors.buttonBg);
                theme.colors.buttonFg  = colorFromHexString(t.colors.getButtonFg(),  theme.colors.buttonFg);
                theme.colors.shadow    = colorFromHexString(t.colors.getShadow(),    theme.colors.shadow);
                theme.colors.dangerBg  = colorFromHexString(t.colors.getDangerBg(),  theme.colors.dangerBg);
                theme.colors.dangerFg  = colorFromHexString(t.colors.getDangerFg(),  theme.colors.dangerFg);
                theme.fonts.header           = montserratFont(t.fonts.getHeader());
                theme.fonts.subheader        = montserratFont(t.fonts.getSubheader());
                theme.fonts.contentHeader    = montserratFont(t.fonts.getContentHeader());
                theme.fonts.contentSubheader = montserratFont(t.fonts.getContentSubheader());
                theme.fonts.content          = montserratFont(t.fonts.getContent());
                break;
            }
        }
        s_ui.setTheme(theme);
    }

    s_wifi      = std::make_unique<AppWIFI>(*s_cfg);
    s_wifi_flow = std::make_unique<lightinator::ui::WifiConfigFlow>(s_ui, *s_wifi);
    s_net_ui_binder = std::make_unique<lightinator::NetworkUiBinder>(
        s_ui, *s_wifi, s_wifi_flow.get(), s_ui_runtime);

    // Bring up hardware (backlight, display, touch).
    const auto caps = s_hw.init();
    if (!caps.displayOk) {
        ESP_LOGE(TAG, "Hardware init failed — display not ready");
        return;
    }

    // Initialise LVGL UI under lock.
    s_ui_runtime.runOnUiThread([&]() {
        if (!s_ui.init()) {
            ESP_LOGE(TAG, "UI init failed");
        }
    });

    // Register WiFi->UI callbacks, start networking, sync initial state.
    s_net_ui_binder->bind();
    s_wifi->init();
    s_ui_runtime.runOnUiThread([&]() { s_net_ui_binder->syncState(); });

    if (s_wifi_flow) {
        s_wifi_flow->startIfNeeded();
    }

    // Start periodic LVGL tick.
    s_ui_runtime.start(s_ui);

    ESP_LOGI(TAG, "=== init() complete ===");
}


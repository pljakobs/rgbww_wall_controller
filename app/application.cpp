#include <SmingCore.h>
#include <esp_log.h>

#include <cctype>
#include <cstdio>

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

namespace {

using lightinator::ui::core::UiTheme;

String colorToHexString(lv_color_t color)
{
    const uint32_t color32 = lv_color_to32(color);
    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02X%02X%02X",
                  static_cast<unsigned>((color32 >> 16U) & 0xFFU),
                  static_cast<unsigned>((color32 >> 8U) & 0xFFU),
                  static_cast<unsigned>(color32 & 0xFFU));
    return String(buf);
}

int fontSizeFromPtr(const lv_font_t* font)
{
    if (font == &lv_font_montserrat_14) return 14;
    if (font == &lv_font_montserrat_16) return 16;
    if (font == &lv_font_montserrat_22) return 22;
    if (font == &lv_font_montserrat_24) return 24;
    if (font == &lv_font_montserrat_34) return 34;
    if (font == &lv_font_montserrat_36) return 36;
    return 16;
}

String slugFromName(const String& name)
{
    String out;
    out.reserve(name.length());
    bool prevUnderscore = false;
    for (size_t i = 0; i < name.length(); ++i) {
        const char c = name[i];
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            prevUnderscore = false;
        } else if (!prevUnderscore) {
            out += '_';
            prevUnderscore = true;
        }
    }
    out.trim();
    while (out.startsWith("_")) {
        out.remove(0, 1);
    }
    while (out.endsWith("_")) {
        out.remove(out.length() - 1, 1);
    }
    if (out.length() == 0) {
        out = "custom_theme";
    }
    return out;
}

bool saveThemeSchema(UiTheme theme)
{
    if (!s_cfg || !s_data) {
        return false;
    }
    theme.name.trim();
    if (theme.name.length() == 0) {
        return false;
    }
    if (theme.id.length() == 0) {
        theme.id = slugFromName(theme.name);
    }

    AppData::Root dataRoot(*s_data);
    if (auto update = dataRoot.update()) {
        auto& themes = update.themes;
        unsigned existingIndex = themes.getItemCount();
        for (unsigned i = 0; i < themes.getItemCount(); ++i) {
            const auto item = themes[i];
            if (item.getId() == theme.id || item.getName() == theme.name) {
                existingIndex = i;
                break;
            }
        }

        auto item = (existingIndex < themes.getItemCount()) ? themes[existingIndex] : themes.addItem();
        item.setId(theme.id);
        item.setName(theme.name);
        item.setMode(theme.mode.equalsIgnoreCase("light") ? 0 : 1);
        item.setHeaderHeight(static_cast<int32_t>(theme.headerHeight));

        item.colors.setHeaderBg(colorToHexString(theme.colors.headerBg));
        item.colors.setHeaderFg(colorToHexString(theme.colors.headerFg));
        item.colors.setContentBg(colorToHexString(theme.colors.contentBg));
        item.colors.setContentFg(colorToHexString(theme.colors.contentFg));
        item.colors.setButtonBg(colorToHexString(theme.colors.buttonBg));
        item.colors.setButtonFg(colorToHexString(theme.colors.buttonFg));
        item.colors.setShadow(colorToHexString(theme.colors.shadow));
        item.colors.setDangerBg(colorToHexString(theme.colors.dangerBg));
        item.colors.setDangerFg(colorToHexString(theme.colors.dangerFg));

        item.fonts.setHeader(fontSizeFromPtr(theme.fonts.header));
        item.fonts.setSubheader(fontSizeFromPtr(theme.fonts.subheader));
        item.fonts.setContentHeader(fontSizeFromPtr(theme.fonts.contentHeader));
        item.fonts.setContentSubheader(fontSizeFromPtr(theme.fonts.contentSubheader));
        item.fonts.setContent(fontSizeFromPtr(theme.fonts.content));
    } else {
        return false;
    }

    AppConfig::Root configRoot(*s_cfg);
    if (auto cfgUpdate = configRoot.update()) {
        cfgUpdate.setActiveTheme(theme.id);
    } else {
        return false;
    }

    s_ui.setTheme(theme);
    return true;
}

std::vector<UiTheme> loadThemeSchemas()
{
    std::vector<UiTheme> out;
    if (!s_data) {
        out.push_back(lightinator::ui::core::nordicDarkTheme());
        return out;
    }

    AppData::Root dataRoot(*s_data);
    for (auto t : dataRoot.themes) {
        UiTheme theme = lightinator::ui::core::nordicDarkTheme();
        theme.id           = t.getId();
        theme.name         = t.getName();
        theme.mode         = (t.getMode() == 0) ? "light" : "dark";
        theme.headerHeight = static_cast<lv_coord_t>(t.getHeaderHeight());
        theme.colors.headerBg  = lightinator::ui::core::colorFromHexString(t.colors.getHeaderBg(),  theme.colors.headerBg);
        theme.colors.headerFg  = lightinator::ui::core::colorFromHexString(t.colors.getHeaderFg(),  theme.colors.headerFg);
        theme.colors.contentBg = lightinator::ui::core::colorFromHexString(t.colors.getContentBg(), theme.colors.contentBg);
        theme.colors.contentFg = lightinator::ui::core::colorFromHexString(t.colors.getContentFg(), theme.colors.contentFg);
        theme.colors.buttonBg  = lightinator::ui::core::colorFromHexString(t.colors.getButtonBg(),  theme.colors.buttonBg);
        theme.colors.buttonFg  = lightinator::ui::core::colorFromHexString(t.colors.getButtonFg(),  theme.colors.buttonFg);
        theme.colors.shadow    = lightinator::ui::core::colorFromHexString(t.colors.getShadow(),    theme.colors.shadow);
        theme.colors.dangerBg  = lightinator::ui::core::colorFromHexString(t.colors.getDangerBg(),  theme.colors.dangerBg);
        theme.colors.dangerFg  = lightinator::ui::core::colorFromHexString(t.colors.getDangerFg(),  theme.colors.dangerFg);
        theme.fonts.header           = lightinator::ui::core::montserratFont(t.fonts.getHeader());
        theme.fonts.subheader        = lightinator::ui::core::montserratFont(t.fonts.getSubheader());
        theme.fonts.contentHeader    = lightinator::ui::core::montserratFont(t.fonts.getContentHeader());
        theme.fonts.contentSubheader = lightinator::ui::core::montserratFont(t.fonts.getContentSubheader());
        theme.fonts.content          = lightinator::ui::core::montserratFont(t.fonts.getContent());
        out.push_back(theme);
    }

    if (out.empty()) {
        out.push_back(lightinator::ui::core::nordicDarkTheme());
    }
    return out;
}

} // namespace

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
        AppConfig::Root cfgRoot(*s_cfg);
        AppData::Root dataRoot(*s_data);
        String activeId = cfgRoot.getActiveTheme();
        for (auto t : dataRoot.themes) {
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
    s_ui.setOnThemeSaveRequested([](const UiTheme& theme) {
        return saveThemeSchema(theme);
    });
    s_ui.setOnThemeListRequested([]() {
        return loadThemeSchemas();
    });

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


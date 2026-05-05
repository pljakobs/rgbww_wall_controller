#pragma once

#include <lvgl.h>
#include <WString.h>

namespace lightinator::ui::core {

struct UiThemeColors {
    lv_color_t headerBg          = lv_color_hex(0x2751a5);
    lv_color_t headerFg          = lv_color_hex(0xD8DEE9);
    lv_color_t contentBg         = lv_color_hex(0x2E3440);
    lv_color_t contentFg         = lv_color_hex(0xD8DEE9);
    lv_color_t buttonBg          = lv_color_hex(0x4C566A);
    lv_color_t buttonFg          = lv_color_hex(0xECEFF4);
    lv_color_t shadow            = lv_color_hex(0x000000);
    lv_color_t dangerBg          = lv_color_hex(0xBF616A);
    lv_color_t dangerFg          = lv_color_hex(0xECEFF4);
};

struct UiThemeFonts {
    const lv_font_t* header           = &lv_font_montserrat_36;
    const lv_font_t* subheader        = &lv_font_montserrat_24;
    const lv_font_t* contentHeader    = &lv_font_montserrat_22;
    const lv_font_t* contentSubheader = &lv_font_montserrat_16;
    const lv_font_t* content          = &lv_font_montserrat_14;
};

struct UiTheme {
    String         id;
    String         name;
    String         mode         = "dark";
    lv_coord_t     headerHeight = 60;
    bool           isBuiltin    = false;  ///< true for flash-stored built-in themes
    UiThemeColors  colors;
    UiThemeFonts   fonts;
};

/// Returns the built-in Nordic Dark theme (used as fallback before config loads).
inline UiTheme nordicDarkTheme()
{
    UiTheme t;
    t.id   = "nordic_dark";
    t.name = "Nordic Dark";
    t.mode = "dark";
    // headerHeight, colors, fonts all use struct defaults (Nordic Dark palette)
    return t;
}

/// Parse a hex color string (#RRGGBB) to lv_color_t.
/// Falls back to the provided default if the string is empty or malformed.
inline lv_color_t colorFromHexString(const String& hex, lv_color_t fallback)
{
    if (hex.length() != 7 || hex[0] != '#') {
        return fallback;
    }
    unsigned long rgb = strtoul(hex.c_str() + 1, nullptr, 16);
    return lv_color_hex(static_cast<uint32_t>(rgb));
}

/// Map a font height (pixels) to the nearest available lv_font_montserrat_* pointer.
/// Available sizes: 14, 16, 22, 24, 34, 36.
inline const lv_font_t* montserratFont(int height)
{
    if (height <= 15) return &lv_font_montserrat_14;
    if (height <= 19) return &lv_font_montserrat_16;
    if (height <= 23) return &lv_font_montserrat_22;
    if (height <= 29) return &lv_font_montserrat_24;
    if (height <= 35) return &lv_font_montserrat_34;
    return &lv_font_montserrat_36;
}

} // namespace lightinator::ui::core

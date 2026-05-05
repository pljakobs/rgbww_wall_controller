#include "builtin_themes.h"

#include <ArduinoJson.h>
#include <FlashString/String.hpp>

using lightinator::ui::core::UiTheme;
using lightinator::ui::core::nordicDarkTheme;
using lightinator::ui::core::colorFromHexString;
using lightinator::ui::core::montserratFont;

// ---------------------------------------------------------------------------
// Built-in theme definitions.
//
// Mapping rules for Material themes (Vuetify palette):
//   headerBg  = color-darken-3
//   headerFg  = color-lighten-5
//   contentBg = base hue, HSV S=10%, V=10%  (near-black with colour tint)
//   contentFg = color-lighten-4
//   buttonBg  = color-accent-2
//   buttonFg  = color-lighten-4
//   shadow    = color-darken-4 with HSV V forced to 25%
//   dangerBg  = color-accent-3
//   dangerFg  = color-accent-1
//
// To add a theme: copy a block, edit the JSON, add to loadBuiltinThemes().
// User-edited copies in ConfigDB shadow these by id at runtime.
// ---------------------------------------------------------------------------

// -- Legacy / hand-crafted themes -------------------------------------------

DEFINE_FSTR_LOCAL(kNordicDarkJson,
    "{\"id\":\"nordic_dark\",\"name\":\"Nordic Dark\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#2751a5\",\"headerFg\":\"#D8DEE9\","
        "\"contentBg\":\"#2E3440\",\"contentFg\":\"#D8DEE9\","
        "\"buttonBg\":\"#4C566A\",\"buttonFg\":\"#ECEFF4\","
        "\"shadow\":\"#3B4252\",\"dangerBg\":\"#BF616A\",\"dangerFg\":\"#ECEFF4\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kClassicLightJson,
    "{\"id\":\"classic_light\",\"name\":\"Classic Light\",\"mode\":\"light\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#ECEFF4\",\"headerFg\":\"#2E3440\","
        "\"contentBg\":\"#FFFFFF\",\"contentFg\":\"#2E3440\","
        "\"buttonBg\":\"#5E81AC\",\"buttonFg\":\"#ECEFF4\","
        "\"shadow\":\"#D8DEE9\",\"dangerBg\":\"#BF616A\",\"dangerFg\":\"#ECEFF4\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kHighContrastJson,
    "{\"id\":\"high_contrast\",\"name\":\"High Contrast\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#000000\",\"headerFg\":\"#FFFFFF\","
        "\"contentBg\":\"#000000\",\"contentFg\":\"#FFFFFF\","
        "\"buttonBg\":\"#222222\",\"buttonFg\":\"#FFFFFF\","
        "\"shadow\":\"#555555\",\"dangerBg\":\"#FF0000\",\"dangerFg\":\"#FFFFFF\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

// -- Material colour families -----------------------------------------------

DEFINE_FSTR_LOCAL(kRedJson,
    "{\"id\":\"red\",\"name\":\"Red\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#C62828\",\"headerFg\":\"#FFEBEE\","
        "\"contentBg\":\"#1A1717\",\"contentFg\":\"#FFCDD2\","
        "\"buttonBg\":\"#FF5252\",\"buttonFg\":\"#FFCDD2\","
        "\"shadow\":\"#400A0A\",\"dangerBg\":\"#FF1744\",\"dangerFg\":\"#FF8A80\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kPinkJson,
    "{\"id\":\"pink\",\"name\":\"Pink\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#AD1457\",\"headerFg\":\"#FCE4EC\","
        "\"contentBg\":\"#1A1718\",\"contentFg\":\"#F8BBD0\","
        "\"buttonBg\":\"#FF4081\",\"buttonFg\":\"#F8BBD0\","
        "\"shadow\":\"#400725\",\"dangerBg\":\"#F50057\",\"dangerFg\":\"#FF80AB\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kPurpleJson,
    "{\"id\":\"purple\",\"name\":\"Purple\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#6A1B9A\",\"headerFg\":\"#F3E5F5\","
        "\"contentBg\":\"#191718\",\"contentFg\":\"#E1BEE7\","
        "\"buttonBg\":\"#E040FB\",\"buttonFg\":\"#E1BEE7\","
        "\"shadow\":\"#220940\",\"dangerBg\":\"#D500F9\",\"dangerFg\":\"#EA80FC\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kDeepPurpleJson,
    "{\"id\":\"deep_purple\",\"name\":\"Deep Purple\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#4527A0\",\"headerFg\":\"#EDE7F6\","
        "\"contentBg\":\"#18171A\",\"contentFg\":\"#D1C4E9\","
        "\"buttonBg\":\"#7C4DFF\",\"buttonFg\":\"#D1C4E9\","
        "\"shadow\":\"#150C40\",\"dangerBg\":\"#651FFF\",\"dangerFg\":\"#B388FF\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kIndigoJson,
    "{\"id\":\"indigo\",\"name\":\"Indigo\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#283593\",\"headerFg\":\"#E8EAF6\","
        "\"contentBg\":\"#17171A\",\"contentFg\":\"#C5CAE9\","
        "\"buttonBg\":\"#536DFE\",\"buttonFg\":\"#C5CAE9\","
        "\"shadow\":\"#0D1140\",\"dangerBg\":\"#3D5AFE\",\"dangerFg\":\"#8C9EFF\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kBlueJson,
    "{\"id\":\"blue\",\"name\":\"Blue\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#1565C0\",\"headerFg\":\"#E3F2FD\","
        "\"contentBg\":\"#17181A\",\"contentFg\":\"#BBDEFB\","
        "\"buttonBg\":\"#448AFF\",\"buttonFg\":\"#BBDEFB\","
        "\"shadow\":\"#051C40\",\"dangerBg\":\"#2979FF\",\"dangerFg\":\"#82B1FF\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kLightBlueJson,
    "{\"id\":\"light_blue\",\"name\":\"Light Blue\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#0277BD\",\"headerFg\":\"#E1F5FE\","
        "\"contentBg\":\"#17191A\",\"contentFg\":\"#B3E5FC\","
        "\"buttonBg\":\"#40C4FF\",\"buttonFg\":\"#B3E5FC\","
        "\"shadow\":\"#002440\",\"dangerBg\":\"#00B0FF\",\"dangerFg\":\"#80D8FF\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kCyanJson,
    "{\"id\":\"cyan\",\"name\":\"Cyan\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#00838F\",\"headerFg\":\"#E0F7FA\","
        "\"contentBg\":\"#17191A\",\"contentFg\":\"#B2EBF2\","
        "\"buttonBg\":\"#18FFFF\",\"buttonFg\":\"#B2EBF2\","
        "\"shadow\":\"#003D40\",\"dangerBg\":\"#00E5FF\",\"dangerFg\":\"#84FFFF\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kTealJson,
    "{\"id\":\"teal\",\"name\":\"Teal\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#00695C\",\"headerFg\":\"#E0F2F1\","
        "\"contentBg\":\"#171A19\",\"contentFg\":\"#B2DFDB\","
        "\"buttonBg\":\"#64FFDA\",\"buttonFg\":\"#B2DFDB\","
        "\"shadow\":\"#004035\",\"dangerBg\":\"#1DE9B6\",\"dangerFg\":\"#A7FFEB\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kGreenJson,
    "{\"id\":\"green\",\"name\":\"Green\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#2E7D32\",\"headerFg\":\"#E8F5E9\","
        "\"contentBg\":\"#171A17\",\"contentFg\":\"#C8E6C9\","
        "\"buttonBg\":\"#69F0AE\",\"buttonFg\":\"#C8E6C9\","
        "\"shadow\":\"#124016\",\"dangerBg\":\"#00E676\",\"dangerFg\":\"#B9F6CA\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kLightGreenJson,
    "{\"id\":\"light_green\",\"name\":\"Light Green\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#558B2F\",\"headerFg\":\"#F1F8E9\","
        "\"contentBg\":\"#181A17\",\"contentFg\":\"#DCEDC8\","
        "\"buttonBg\":\"#B2FF59\",\"buttonFg\":\"#DCEDC8\","
        "\"shadow\":\"#1F4012\",\"dangerBg\":\"#76FF03\",\"dangerFg\":\"#CCFF90\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kLimeJson,
    "{\"id\":\"lime\",\"name\":\"Lime\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#9E9D24\",\"headerFg\":\"#F9FBE7\","
        "\"contentBg\":\"#191A17\",\"contentFg\":\"#F0F4C3\","
        "\"buttonBg\":\"#EEFF41\",\"buttonFg\":\"#F0F4C3\","
        "\"shadow\":\"#403B0B\",\"dangerBg\":\"#C6FF00\",\"dangerFg\":\"#F4FF81\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kYellowJson,
    "{\"id\":\"yellow\",\"name\":\"Yellow\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#F9A825\",\"headerFg\":\"#FFFDE7\","
        "\"contentBg\":\"#1A1917\",\"contentFg\":\"#FFF9C4\","
        "\"buttonBg\":\"#FFFF00\",\"buttonFg\":\"#FFF9C4\","
        "\"shadow\":\"#402106\",\"dangerBg\":\"#FFEA00\",\"dangerFg\":\"#FFFF8D\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kAmberJson,
    "{\"id\":\"amber\",\"name\":\"Amber\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#FF8F00\",\"headerFg\":\"#FFF8E1\","
        "\"contentBg\":\"#1A1917\",\"contentFg\":\"#FFECB3\","
        "\"buttonBg\":\"#FFD740\",\"buttonFg\":\"#FFECB3\","
        "\"shadow\":\"#401C00\",\"dangerBg\":\"#FFC400\",\"dangerFg\":\"#FFE57F\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kOrangeJson,
    "{\"id\":\"orange\",\"name\":\"Orange\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#EF6C00\",\"headerFg\":\"#FFF3E0\","
        "\"contentBg\":\"#1A1817\",\"contentFg\":\"#FFE0B2\","
        "\"buttonBg\":\"#FFAB40\",\"buttonFg\":\"#FFE0B2\","
        "\"shadow\":\"#401700\",\"dangerBg\":\"#FF9100\",\"dangerFg\":\"#FFD180\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

DEFINE_FSTR_LOCAL(kDeepOrangeJson,
    "{\"id\":\"deep_orange\",\"name\":\"Deep Orange\",\"mode\":\"dark\","
    "\"headerHeight\":60,"
    "\"colors\":{"
        "\"headerBg\":\"#D84315\",\"headerFg\":\"#FBE9E7\","
        "\"contentBg\":\"#1A1717\",\"contentFg\":\"#FFCCBC\","
        "\"buttonBg\":\"#FF6E40\",\"buttonFg\":\"#FFCCBC\","
        "\"shadow\":\"#401204\",\"dangerBg\":\"#FF3D00\",\"dangerFg\":\"#FF9E80\"},"
    "\"fonts\":{"
        "\"header\":36,\"subheader\":24,\"contentHeader\":22,"
        "\"contentSubheader\":16,\"content\":14}}"
)

// ---------------------------------------------------------------------------
// Parser — copies flash string to RAM, then deserialises with ArduinoJson 6.
// ---------------------------------------------------------------------------

static UiTheme parseThemeJson(const FSTR::String& fstr)
{
    UiTheme t = nordicDarkTheme();

    String json = fstr; // copy from flash to RAM

    StaticJsonDocument<768> doc;
    if (!Json::deserialize(doc, json)) {
        return t;
    }

    if (doc.containsKey("id"))           t.id           = (const char*)doc["id"];
    if (doc.containsKey("name"))         t.name         = (const char*)doc["name"];
    if (doc.containsKey("mode"))         t.mode         = (const char*)doc["mode"];
    if (doc.containsKey("headerHeight")) t.headerHeight = static_cast<lv_coord_t>(doc["headerHeight"].as<int>());

    JsonObjectConst colors = doc["colors"];
    if (!colors.isNull()) {
        t.colors.headerBg  = colorFromHexString(colors["headerBg"]  | "", t.colors.headerBg);
        t.colors.headerFg  = colorFromHexString(colors["headerFg"]  | "", t.colors.headerFg);
        t.colors.contentBg = colorFromHexString(colors["contentBg"] | "", t.colors.contentBg);
        t.colors.contentFg = colorFromHexString(colors["contentFg"] | "", t.colors.contentFg);
        t.colors.buttonBg  = colorFromHexString(colors["buttonBg"]  | "", t.colors.buttonBg);
        t.colors.buttonFg  = colorFromHexString(colors["buttonFg"]  | "", t.colors.buttonFg);
        t.colors.shadow    = colorFromHexString(colors["shadow"]    | "", t.colors.shadow);
        t.colors.dangerBg  = colorFromHexString(colors["dangerBg"]  | "", t.colors.dangerBg);
        t.colors.dangerFg  = colorFromHexString(colors["dangerFg"]  | "", t.colors.dangerFg);
    }

    JsonObjectConst fonts = doc["fonts"];
    if (!fonts.isNull()) {
        t.fonts.header           = montserratFont(fonts["header"]           | 36);
        t.fonts.subheader        = montserratFont(fonts["subheader"]        | 24);
        t.fonts.contentHeader    = montserratFont(fonts["contentHeader"]    | 22);
        t.fonts.contentSubheader = montserratFont(fonts["contentSubheader"] | 16);
        t.fonts.content          = montserratFont(fonts["content"]          | 14);
    }

    return t;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::vector<UiTheme> loadBuiltinThemes()
{
    return {
        // Legacy / hand-crafted
        parseThemeJson(kNordicDarkJson),
        parseThemeJson(kClassicLightJson),
        parseThemeJson(kHighContrastJson),
        // Material colour families
        parseThemeJson(kRedJson),
        parseThemeJson(kPinkJson),
        parseThemeJson(kPurpleJson),
        parseThemeJson(kDeepPurpleJson),
        parseThemeJson(kIndigoJson),
        parseThemeJson(kBlueJson),
        parseThemeJson(kLightBlueJson),
        parseThemeJson(kCyanJson),
        parseThemeJson(kTealJson),
        parseThemeJson(kGreenJson),
        parseThemeJson(kLightGreenJson),
        parseThemeJson(kLimeJson),
        parseThemeJson(kYellowJson),
        parseThemeJson(kAmberJson),
        parseThemeJson(kOrangeJson),
        parseThemeJson(kDeepOrangeJson),
    };
}

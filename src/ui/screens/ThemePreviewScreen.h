#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <lvgl.h>
#include <lvglCpp.h>

#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"
#include "ui/widgets/HsvColorPicker.h"

namespace lightinator::ui::screens {

class ThemePreviewScreen : public core::Screen {
public:
    ThemePreviewScreen(const core::UiTheme& theme, const String& suggestedName);

    void mount(lv_obj_t* parent) override;
    void applyLiveTheme(const core::UiTheme& theme);

    void setOnCloseRequested(std::function<void()> callback);
    void setOnSaveRequested(std::function<bool(const core::UiTheme&)> callback);
    void setOnThemeApplyRequested(std::function<void(const core::UiTheme&)> callback);

private:
    static void onSaveButtonEvent(lv_event_t* event);
    static void onTextAreaEvent(lv_event_t* event);
    static void onKeyboardEvent(lv_event_t* event);
    static void onColorInputChanged(lv_event_t* event);
    static void onColorFieldClicked(lv_event_t* event);
    static void onFontDropdownChanged(lv_event_t* event);
    static void onColorPickerCancelEvent(lv_event_t* event);

    bool collectThemeFromInputs(core::UiTheme& outTheme) const;
    void applyThemeToInputs(const core::UiTheme& theme);
    void hideKeyboard();
    void showColorPickerForField(lv_obj_t* field, lv_obj_t* swatch);
    void hideColorPicker();
    void updateSingleSwatch(lv_obj_t* field, lv_obj_t* swatch);

    void buildColorsTab(lv_obj_t* tab);
    void buildFontsTab(lv_obj_t* tab);

    void addColorRow(lv_obj_t* parent, const char* name, lv_color_t color,
                     lv_obj_t** fieldOut, lv_obj_t** swatchOut);
    void addFontRow(lv_obj_t* parent, const char* name, int initialSize,
                    lv_obj_t** sliderOut);

    core::UiTheme theme_;
    String suggestedName_;
    std::function<void()> onCloseRequested_;
    std::function<bool(const core::UiTheme&)> onSaveRequested_;
    std::function<void(const core::UiTheme&)> onThemeApplyRequested_;

    lv_obj_t* schemaNameField_ = nullptr;
    lv_obj_t* statusLabel_ = nullptr;
    lv_obj_t* keyboard_ = nullptr;
    lv_obj_t* colorPickerOverlay_ = nullptr;
    lv_obj_t* activeColorField_ = nullptr;
    lv_obj_t* activeColorSwatch_ = nullptr;
    String activeColorOriginalText_;
    std::unique_ptr<widgets::HsvColorPicker> colorPickerWidget_;

    lv_obj_t* headerBgField_ = nullptr;
    lv_obj_t* headerFgField_ = nullptr;
    lv_obj_t* contentBgField_ = nullptr;
    lv_obj_t* contentFgField_ = nullptr;
    lv_obj_t* buttonBgField_ = nullptr;
    lv_obj_t* buttonFgField_ = nullptr;
    lv_obj_t* shadowField_ = nullptr;
    lv_obj_t* dangerBgField_ = nullptr;
    lv_obj_t* dangerFgField_ = nullptr;

    lv_obj_t* headerBgSwatch_ = nullptr;
    lv_obj_t* headerFgSwatch_ = nullptr;
    lv_obj_t* contentBgSwatch_ = nullptr;
    lv_obj_t* contentFgSwatch_ = nullptr;
    lv_obj_t* buttonBgSwatch_ = nullptr;
    lv_obj_t* buttonFgSwatch_ = nullptr;
    lv_obj_t* shadowSwatch_ = nullptr;
    lv_obj_t* dangerBgSwatch_ = nullptr;
    lv_obj_t* dangerFgSwatch_ = nullptr;

    lv_obj_t* headerFontDropdown_ = nullptr;
    lv_obj_t* subheaderFontDropdown_ = nullptr;
    lv_obj_t* contentHeaderFontDropdown_ = nullptr;
    lv_obj_t* contentSubheaderFontDropdown_ = nullptr;
    lv_obj_t* contentFontDropdown_ = nullptr;

    std::unique_ptr<DecoratedScreen> decorated_;
};

} // namespace lightinator::ui::screens

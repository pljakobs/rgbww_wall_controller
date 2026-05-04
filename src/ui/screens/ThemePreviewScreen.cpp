#include "ui/screens/ThemePreviewScreen.h"

#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>

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

bool parseHexColor(const char* text, lv_color_t& out)
{
    if (text == nullptr || text[0] != '#') {
        return false;
    }
    if (std::strlen(text) != 7) {
        return false;
    }
    for (size_t i = 1; i < 7; ++i) {
        if (!std::isxdigit(static_cast<unsigned char>(text[i]))) {
            return false;
        }
    }
    const uint32_t rgb = static_cast<uint32_t>(strtoul(text + 1, nullptr, 16));
    out = lv_color_hex(rgb);
    return true;
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

lightinator::ui::core::HsvColor toCoreHsv(lv_color_t color)
{
    const lv_color_hsv_t hsv = lv_color_to_hsv(color);
    return {
        lightinator::ui::core::fromLvHue(hsv.h),
        lightinator::ui::core::fromLvPercent(hsv.s),
        lightinator::ui::core::fromLvPercent(hsv.v),
    };
}

lv_color_t toLvColor(const lightinator::ui::core::HsvColor& color)
{
    return lv_color_hsv_to_rgb(
        lightinator::ui::core::toLvHue(color.h),
        lightinator::ui::core::toLvPercent(color.s),
        lightinator::ui::core::toLvPercent(color.v));
}

} // namespace

namespace lightinator::ui::screens {

ThemePreviewScreen::ThemePreviewScreen(const core::UiTheme& theme)
    : theme_(theme)
{
}

void ThemePreviewScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void ThemePreviewScreen::setOnSaveRequested(std::function<bool(const core::UiTheme&)> callback)
{
    onSaveRequested_ = std::move(callback);
}

void ThemePreviewScreen::setOnThemeListRequested(std::function<std::vector<core::UiTheme>()> callback)
{
    onThemeListRequested_ = std::move(callback);
}

void ThemePreviewScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text      = "Theme Editor";
    header.showClose = true;
    header.height    = theme_.headerHeight;
    header.color     = theme_.colors.headerBg;
    header.font      = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
    decorated_->setOnCloseRequested([this]() {
        if (onCloseRequested_) {
            onCloseRequested_();
        }
    });
    decorated_->mount(parent);
    setRoot(decorated_->root());

    lv_obj_t* body = decorated_->bodySlot();

    lv_obj_t* layout = lv_obj_create(body);
    lv_obj_set_size(layout, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(layout, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(layout, 0, 0);
    lv_obj_set_style_pad_all(layout, 0, 0);
    lv_obj_set_style_pad_row(layout, 8, 0);
    lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* actions = lv_obj_create(layout);
    lv_obj_set_width(actions, lv_pct(100));
    lv_obj_set_height(actions, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(actions, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(actions, 0, 0);
    lv_obj_set_style_pad_left(actions, 8, 0);
    lv_obj_set_style_pad_right(actions, 8, 0);
    lv_obj_set_style_pad_top(actions, 8, 0);
    lv_obj_set_style_pad_bottom(actions, 0, 0);
    lv_obj_set_style_pad_column(actions, 8, 0);
    lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(actions, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    schemaNameField_ = lv_textarea_create(actions);
    lv_obj_set_height(schemaNameField_, 44);
    lv_obj_set_flex_grow(schemaNameField_, 1);
    lv_textarea_set_one_line(schemaNameField_, true);
    lv_textarea_set_max_length(schemaNameField_, 32);
    lv_textarea_set_text(schemaNameField_, theme_.name.c_str());
    lv_obj_set_style_text_font(schemaNameField_, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(schemaNameField_, theme_.colors.contentFg, 0);
    lv_obj_set_style_bg_color(schemaNameField_, theme_.colors.contentBg, 0);
    lv_obj_set_style_border_color(schemaNameField_, theme_.colors.shadow, 0);
    lv_obj_add_event_cb(schemaNameField_, onTextAreaEvent, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(schemaNameField_, onTextAreaEvent, LV_EVENT_FOCUSED, this);
    lv_obj_add_event_cb(schemaNameField_, onTextAreaEvent, LV_EVENT_READY, this);
    lv_obj_add_event_cb(schemaNameField_, onTextAreaEvent, LV_EVENT_CANCEL, this);

    lv_obj_t* saveButton = lv_btn_create(actions);
    lv_obj_set_size(saveButton, 138, 44);
    lv_obj_set_style_bg_color(saveButton, theme_.colors.buttonBg, 0);
    lv_obj_set_style_text_color(saveButton, theme_.colors.buttonFg, 0);
    lv_obj_set_style_border_width(saveButton, 0, 0);
    lv_obj_add_event_cb(saveButton, onSaveButtonEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* saveLabel = lv_label_create(saveButton);
    lv_label_set_text_static(saveLabel, "Save schema");
    lv_obj_set_style_text_font(saveLabel, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(saveLabel, theme_.colors.buttonFg, 0);
    lv_obj_center(saveLabel);

    statusLabel_ = lv_label_create(layout);
    lv_label_set_text_static(statusLabel_, "Edit and save to app-data");
    lv_obj_set_style_text_font(statusLabel_, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(statusLabel_, theme_.colors.contentFg, 0);
    lv_obj_set_style_pad_left(statusLabel_, 10, 0);

    lv_obj_t* selectorRow = lv_obj_create(layout);
    lv_obj_set_width(selectorRow, lv_pct(100));
    lv_obj_set_height(selectorRow, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(selectorRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(selectorRow, 0, 0);
    lv_obj_set_style_pad_left(selectorRow, 8, 0);
    lv_obj_set_style_pad_right(selectorRow, 8, 0);
    lv_obj_set_style_pad_top(selectorRow, 0, 0);
    lv_obj_set_style_pad_bottom(selectorRow, 0, 0);
    lv_obj_set_style_pad_column(selectorRow, 8, 0);
    lv_obj_set_flex_flow(selectorRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(selectorRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* selectorLabel = lv_label_create(selectorRow);
    lv_label_set_text_static(selectorLabel, "Load scheme:");
    lv_obj_set_style_text_font(selectorLabel, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(selectorLabel, theme_.colors.contentFg, 0);

    schemeDropdown_ = lv_dropdown_create(selectorRow);
    lv_obj_set_height(schemeDropdown_, 44);
    lv_obj_set_flex_grow(schemeDropdown_, 1);
    lv_obj_set_style_text_font(schemeDropdown_, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(schemeDropdown_, theme_.colors.contentFg, 0);
    lv_obj_set_style_bg_color(schemeDropdown_, theme_.colors.contentBg, 0);
    lv_obj_set_style_border_color(schemeDropdown_, theme_.colors.shadow, 0);

    if (onThemeListRequested_) {
        availableThemes_ = onThemeListRequested_();
    }
    if (availableThemes_.empty()) {
        availableThemes_.push_back(theme_);
    }

    String options;
    int selectedIndex = 0;
    for (size_t i = 0; i < availableThemes_.size(); ++i) {
        if (options.length() != 0) {
            options += "\n";
        }
        options += availableThemes_[i].name;

        if ((theme_.id.length() != 0 && availableThemes_[i].id == theme_.id) ||
            (availableThemes_[i].name == theme_.name)) {
            selectedIndex = static_cast<int>(i);
        }
    }
    lv_dropdown_set_options(schemeDropdown_, options.c_str());
    lv_dropdown_set_selected(schemeDropdown_, static_cast<uint16_t>(selectedIndex));
    lv_obj_add_event_cb(schemeDropdown_, onSchemeDropdownChanged, LV_EVENT_VALUE_CHANGED, this);

    // Tabview filling the entire body slot
    lv_obj_t* tabview = lv_tabview_create(layout, LV_DIR_TOP, 48);
    lv_obj_set_width(tabview, lv_pct(100));
    lv_obj_set_flex_grow(tabview, 1);
    lv_obj_set_style_bg_color(tabview, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(tabview, LV_OPA_COVER, 0);

    // Style the tab buttons bar
    lv_obj_t* tabBtns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tabBtns, theme_.colors.headerBg, 0);
    lv_obj_set_style_bg_opa(tabBtns, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(tabBtns, theme_.colors.headerFg, 0);
    lv_obj_set_style_text_font(tabBtns, theme_.fonts.subheader, 0);
    lv_obj_set_style_border_side(tabBtns, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(tabBtns, theme_.colors.shadow, 0);

    lv_obj_t* colorsTab = lv_tabview_add_tab(tabview, "Colors");
    lv_obj_t* fontsTab  = lv_tabview_add_tab(tabview, "Fonts");

    buildColorsTab(colorsTab);
    buildFontsTab(fontsTab);

    keyboard_ = lv_keyboard_create(decorated_->root());
    lv_obj_set_size(keyboard_, lv_pct(100), lv_pct(45));
    lv_obj_align(keyboard_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_popovers(keyboard_, true);
    lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(keyboard_, onKeyboardEvent, LV_EVENT_READY, this);
    lv_obj_add_event_cb(keyboard_, onKeyboardEvent, LV_EVENT_CANCEL, this);
}

void ThemePreviewScreen::buildColorsTab(lv_obj_t* tab)
{
    lv_obj_set_style_bg_color(tab, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(tab, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(tab, 8, 0);
    lv_obj_set_style_pad_row(tab, 6, 0);
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    addColorRow(tab, "headerBg", theme_.colors.headerBg, &headerBgField_, &headerBgSwatch_);
    addColorRow(tab, "headerFg", theme_.colors.headerFg, &headerFgField_, &headerFgSwatch_);
    addColorRow(tab, "contentBg", theme_.colors.contentBg, &contentBgField_, &contentBgSwatch_);
    addColorRow(tab, "contentFg", theme_.colors.contentFg, &contentFgField_, &contentFgSwatch_);
    addColorRow(tab, "buttonBg", theme_.colors.buttonBg, &buttonBgField_, &buttonBgSwatch_);
    addColorRow(tab, "buttonFg", theme_.colors.buttonFg, &buttonFgField_, &buttonFgSwatch_);
    addColorRow(tab, "shadow", theme_.colors.shadow, &shadowField_, &shadowSwatch_);
    addColorRow(tab, "dangerBg", theme_.colors.dangerBg, &dangerBgField_, &dangerBgSwatch_);
    addColorRow(tab, "dangerFg", theme_.colors.dangerFg, &dangerFgField_, &dangerFgSwatch_);
}

void ThemePreviewScreen::buildFontsTab(lv_obj_t* tab)
{
    lv_obj_set_style_bg_color(tab, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(tab, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(tab, 8, 0);
    lv_obj_set_style_pad_row(tab, 10, 0);
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    addFontRow(tab, "header", fontSizeFromPtr(theme_.fonts.header), &headerSizeSlider_);
    addFontRow(tab, "subheader", fontSizeFromPtr(theme_.fonts.subheader), &subheaderSizeSlider_);
    addFontRow(tab, "contentHeader", fontSizeFromPtr(theme_.fonts.contentHeader), &contentHeaderSizeSlider_);
    addFontRow(tab, "contentSubheader", fontSizeFromPtr(theme_.fonts.contentSubheader), &contentSubheaderSizeSlider_);
    addFontRow(tab, "content", fontSizeFromPtr(theme_.fonts.content), &contentSizeSlider_);
}

void ThemePreviewScreen::addColorRow(lv_obj_t* parent, const char* name, lv_color_t color,
                                     lv_obj_t** fieldOut, lv_obj_t** swatchOut)
{
    // Row container
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 10, 0);

    // Label
    lv_obj_t* label = lv_label_create(row);
    lv_label_set_text_static(label, name);
    lv_obj_set_style_text_font(label, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(label, theme_.colors.contentFg, 0);
    lv_obj_set_width(label, 130);

    lv_obj_t* field = lv_textarea_create(row);
    lv_obj_set_size(field, 106, 36);
    lv_textarea_set_one_line(field, true);
    lv_textarea_set_max_length(field, 7);
    lv_textarea_set_accepted_chars(field, "#0123456789abcdefABCDEF");
    lv_textarea_set_cursor_click_pos(field, false);
    lv_textarea_set_text(field, colorToHexString(color).c_str());
    lv_obj_set_style_text_font(field, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(field, theme_.colors.contentFg, 0);
    lv_obj_set_style_bg_color(field, theme_.colors.contentBg, 0);
    lv_obj_set_style_border_color(field, theme_.colors.shadow, 0);
    lv_obj_add_event_cb(field, onColorFieldClicked, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(field, onColorInputChanged, LV_EVENT_VALUE_CHANGED, this);

    // Color swatch
    lv_obj_t* swatch = lv_obj_create(row);
    lv_obj_set_size(swatch, 36, 36);
    lv_obj_set_style_radius(swatch, 4, 0);
    lv_obj_set_style_bg_color(swatch, color, 0);
    lv_obj_set_style_bg_opa(swatch, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(swatch, 1, 0);
    lv_obj_set_style_border_color(swatch, theme_.colors.shadow, 0);

    if (fieldOut) {
        *fieldOut = field;
    }
    if (swatchOut) {
        *swatchOut = swatch;
    }
}

void ThemePreviewScreen::addFontRow(lv_obj_t* parent, const char* name, int initialSize,
                                    lv_obj_t** sliderOut)
{
    // Row container
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 10, 0);

    lv_obj_t* nameLabel = lv_label_create(row);
    lv_label_set_text_static(nameLabel, name);
    lv_obj_set_width(nameLabel, 150);
    lv_obj_set_style_text_font(nameLabel, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(nameLabel, theme_.colors.contentFg, 0);

    lv_obj_t* slider = lv_slider_create(row);
    lv_obj_set_size(slider, 190, 10);
    lv_slider_set_range(slider, 10, 48);
    lv_slider_set_value(slider, initialSize, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, theme_.colors.shadow, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, theme_.colors.buttonBg, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, theme_.colors.buttonFg, LV_PART_KNOB);

    lv_obj_t* valueLabel = lv_label_create(row);
    lv_label_set_text_fmt(valueLabel, "%d", initialSize);
    lv_obj_set_style_text_font(valueLabel, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(valueLabel, theme_.colors.contentFg, 0);
    lv_obj_add_event_cb(slider, onFontSliderChanged, LV_EVENT_VALUE_CHANGED, valueLabel);

    if (sliderOut) {
        *sliderOut = slider;
    }
}

void ThemePreviewScreen::onSaveButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<ThemePreviewScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    core::UiTheme updatedTheme;
    if (!self->collectThemeFromInputs(updatedTheme)) {
        if (self->statusLabel_) {
            lv_label_set_text_static(self->statusLabel_, "Invalid input: name and #RRGGBB values required");
            lv_obj_set_style_text_color(self->statusLabel_, self->theme_.colors.dangerFg, 0);
        }
        return;
    }

    if (!self->onSaveRequested_) {
        if (self->statusLabel_) {
            lv_label_set_text_static(self->statusLabel_, "Save handler not configured");
            lv_obj_set_style_text_color(self->statusLabel_, self->theme_.colors.dangerFg, 0);
        }
        return;
    }

    if (self->onSaveRequested_(updatedTheme)) {
        self->theme_ = updatedTheme;
        if (self->statusLabel_) {
            lv_label_set_text_static(self->statusLabel_, "Schema saved to app-data");
            lv_obj_set_style_text_color(self->statusLabel_, self->theme_.colors.contentFg, 0);
        }
        self->hideKeyboard();
    } else if (self->statusLabel_) {
        lv_label_set_text_static(self->statusLabel_, "Failed to save schema");
        lv_obj_set_style_text_color(self->statusLabel_, self->theme_.colors.dangerFg, 0);
    }
}

void ThemePreviewScreen::onTextAreaEvent(lv_event_t* event)
{
    auto* self = static_cast<ThemePreviewScreen*>(lv_event_get_user_data(event));
    if (self == nullptr || self->keyboard_ == nullptr) {
        return;
    }

    const lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* target = lv_event_get_target(event);

    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(self->keyboard_, target);
        lv_obj_clear_flag(self->keyboard_, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        self->hideKeyboard();
    }
}

void ThemePreviewScreen::onKeyboardEvent(lv_event_t* event)
{
    auto* self = static_cast<ThemePreviewScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    const lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        self->hideKeyboard();
    }
}

void ThemePreviewScreen::onColorInputChanged(lv_event_t* event)
{
    auto* self = static_cast<ThemePreviewScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    lv_obj_t* field = lv_event_get_target(event);
    if (field == self->headerBgField_) self->updateSingleSwatch(self->headerBgField_, self->headerBgSwatch_);
    else if (field == self->headerFgField_) self->updateSingleSwatch(self->headerFgField_, self->headerFgSwatch_);
    else if (field == self->contentBgField_) self->updateSingleSwatch(self->contentBgField_, self->contentBgSwatch_);
    else if (field == self->contentFgField_) self->updateSingleSwatch(self->contentFgField_, self->contentFgSwatch_);
    else if (field == self->buttonBgField_) self->updateSingleSwatch(self->buttonBgField_, self->buttonBgSwatch_);
    else if (field == self->buttonFgField_) self->updateSingleSwatch(self->buttonFgField_, self->buttonFgSwatch_);
    else if (field == self->shadowField_) self->updateSingleSwatch(self->shadowField_, self->shadowSwatch_);
    else if (field == self->dangerBgField_) self->updateSingleSwatch(self->dangerBgField_, self->dangerBgSwatch_);
    else if (field == self->dangerFgField_) self->updateSingleSwatch(self->dangerFgField_, self->dangerFgSwatch_);
}

void ThemePreviewScreen::onColorFieldClicked(lv_event_t* event)
{
    auto* self = static_cast<ThemePreviewScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    lv_obj_t* field = lv_event_get_target(event);
    lv_obj_t* swatch = nullptr;
    if (field == self->headerBgField_) swatch = self->headerBgSwatch_;
    else if (field == self->headerFgField_) swatch = self->headerFgSwatch_;
    else if (field == self->contentBgField_) swatch = self->contentBgSwatch_;
    else if (field == self->contentFgField_) swatch = self->contentFgSwatch_;
    else if (field == self->buttonBgField_) swatch = self->buttonBgSwatch_;
    else if (field == self->buttonFgField_) swatch = self->buttonFgSwatch_;
    else if (field == self->shadowField_) swatch = self->shadowSwatch_;
    else if (field == self->dangerBgField_) swatch = self->dangerBgSwatch_;
    else if (field == self->dangerFgField_) swatch = self->dangerFgSwatch_;

    if (swatch == nullptr) {
        return;
    }

    self->showColorPickerForField(field, swatch);
}

void ThemePreviewScreen::onFontSliderChanged(lv_event_t* event)
{
    auto* valueLabel = static_cast<lv_obj_t*>(lv_event_get_user_data(event));
    if (valueLabel == nullptr) {
        return;
    }
    lv_obj_t* slider = lv_event_get_target(event);
    lv_label_set_text_fmt(valueLabel, "%ld", lv_slider_get_value(slider));
}

void ThemePreviewScreen::onSchemeDropdownChanged(lv_event_t* event)
{
    auto* self = static_cast<ThemePreviewScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    lv_obj_t* dropdown = lv_event_get_target(event);
    const uint16_t selectedIndex = lv_dropdown_get_selected(dropdown);
    if (selectedIndex >= self->availableThemes_.size()) {
        return;
    }

    self->theme_ = self->availableThemes_[selectedIndex];
    self->applyThemeToInputs(self->theme_);
    self->hideKeyboard();
    if (self->statusLabel_) {
        lv_label_set_text_fmt(self->statusLabel_, "Loaded schema: %s", self->theme_.name.c_str());
        lv_obj_set_style_text_color(self->statusLabel_, self->theme_.colors.contentFg, 0);
    }
}

void ThemePreviewScreen::onColorPickerCancelEvent(lv_event_t* event)
{
    auto* self = static_cast<ThemePreviewScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    if (self->activeColorField_ != nullptr) {
        lv_textarea_set_text(self->activeColorField_, self->activeColorOriginalText_.c_str());
        self->updateSingleSwatch(self->activeColorField_, self->activeColorSwatch_);
    }
    self->hideColorPicker();
}

void ThemePreviewScreen::hideKeyboard()
{
    if (keyboard_ == nullptr) {
        return;
    }
    lv_keyboard_set_textarea(keyboard_, nullptr);
    lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
}

void ThemePreviewScreen::showColorPickerForField(lv_obj_t* field, lv_obj_t* swatch)
{
    if (field == nullptr || swatch == nullptr || decorated_ == nullptr) {
        return;
    }

    hideKeyboard();
    hideColorPicker();

    activeColorField_ = field;
    activeColorSwatch_ = swatch;
    activeColorOriginalText_ = lv_textarea_get_text(field);

    lv_obj_t* root = decorated_->root();
    colorPickerOverlay_ = lv_obj_create(root);
    lv_obj_set_size(colorPickerOverlay_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(colorPickerOverlay_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(colorPickerOverlay_, LV_OPA_40, 0);
    lv_obj_set_style_border_width(colorPickerOverlay_, 0, 0);
    lv_obj_set_style_pad_all(colorPickerOverlay_, 0, 0);

    lv_obj_t* panel = lv_obj_create(colorPickerOverlay_);
    lv_obj_set_size(panel, 320, 430);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, theme_.colors.shadow, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 8, 0);
    lv_obj_set_style_pad_all(panel, 10, 0);
    lv_obj_set_style_pad_row(panel, 8, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    lv_obj_t* title = lv_label_create(panel);
    lv_label_set_text_static(title, "Pick color");
    lv_obj_set_style_text_font(title, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(title, theme_.colors.contentFg, 0);

    lv_obj_t* pickerHost = lv_obj_create(panel);
    lv_obj_set_width(pickerHost, lv_pct(100));
    lv_obj_set_flex_grow(pickerHost, 1);
    lv_obj_set_style_bg_opa(pickerHost, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(pickerHost, 0, 0);
    lv_obj_set_style_pad_all(pickerHost, 0, 0);

    lv_color_t currentColor;
    core::HsvColor initialHsv = {};
    if (parseHexColor(lv_textarea_get_text(field), currentColor)) {
        initialHsv = toCoreHsv(currentColor);
    } else {
        initialHsv = toCoreHsv(theme_.colors.contentFg);
    }

    colorPickerWidget_ = std::make_unique<widgets::HsvColorPicker>();
    colorPickerWidget_->setValue(initialHsv);
    colorPickerWidget_->setOnColorChanged([this](const core::HsvColor& hsvColor) {
        if (activeColorField_ == nullptr) {
            return;
        }
        lv_textarea_set_text(activeColorField_, colorToHexString(toLvColor(hsvColor)).c_str());
        updateSingleSwatch(activeColorField_, activeColorSwatch_);
    });
    colorPickerWidget_->setOnColorCommitted([this](const core::HsvColor& hsvColor) {
        if (activeColorField_ != nullptr) {
            lv_textarea_set_text(activeColorField_, colorToHexString(toLvColor(hsvColor)).c_str());
            updateSingleSwatch(activeColorField_, activeColorSwatch_);
        }
        hideColorPicker();
    });
    colorPickerWidget_->mount(pickerHost);

    lv_obj_t* actions = lv_obj_create(panel);
    lv_obj_set_size(actions, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(actions, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(actions, 0, 0);
    lv_obj_set_style_pad_all(actions, 0, 0);
    lv_obj_set_style_pad_column(actions, 8, 0);
    lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(actions, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* cancelBtn = lv_btn_create(actions);
    lv_obj_set_size(cancelBtn, 96, 40);
    lv_obj_set_style_bg_color(cancelBtn, theme_.colors.shadow, 0);
    lv_obj_set_style_border_width(cancelBtn, 0, 0);
    lv_obj_add_event_cb(cancelBtn, onColorPickerCancelEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
    lv_label_set_text_static(cancelLabel, "Cancel");
    lv_obj_set_style_text_font(cancelLabel, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(cancelLabel, theme_.colors.contentFg, 0);
    lv_obj_center(cancelLabel);

}

void ThemePreviewScreen::hideColorPicker()
{
    if (colorPickerOverlay_) {
        lv_obj_del(colorPickerOverlay_);
        colorPickerOverlay_ = nullptr;
    }
    colorPickerWidget_.reset();
    activeColorOriginalText_ = String();
    activeColorField_ = nullptr;
    activeColorSwatch_ = nullptr;
}

void ThemePreviewScreen::updateSingleSwatch(lv_obj_t* field, lv_obj_t* swatch)
{
    if (field == nullptr || swatch == nullptr) {
        return;
    }
    lv_color_t parsed;
    const char* text = lv_textarea_get_text(field);
    if (parseHexColor(text, parsed)) {
        lv_obj_set_style_bg_color(swatch, parsed, 0);
        lv_obj_set_style_border_color(field, theme_.colors.shadow, 0);
    } else {
        lv_obj_set_style_border_color(field, theme_.colors.dangerBg, 0);
    }
}

bool ThemePreviewScreen::collectThemeFromInputs(core::UiTheme& outTheme) const
{
    if (schemaNameField_ == nullptr) {
        return false;
    }

    const char* nameText = lv_textarea_get_text(schemaNameField_);
    String name = nameText ? String(nameText) : String();
    name.trim();
    if (name.length() == 0) {
        return false;
    }

    outTheme = theme_;
    outTheme.name = name;

    const struct {
        lv_obj_t* field;
        lv_color_t* out;
    } colors[] = {
        {headerBgField_, &outTheme.colors.headerBg},
        {headerFgField_, &outTheme.colors.headerFg},
        {contentBgField_, &outTheme.colors.contentBg},
        {contentFgField_, &outTheme.colors.contentFg},
        {buttonBgField_, &outTheme.colors.buttonBg},
        {buttonFgField_, &outTheme.colors.buttonFg},
        {shadowField_, &outTheme.colors.shadow},
        {dangerBgField_, &outTheme.colors.dangerBg},
        {dangerFgField_, &outTheme.colors.dangerFg},
    };

    for (const auto& entry : colors) {
        if (entry.field == nullptr || entry.out == nullptr) {
            return false;
        }
        lv_color_t parsed;
        if (!parseHexColor(lv_textarea_get_text(entry.field), parsed)) {
            return false;
        }
        *entry.out = parsed;
    }

    if (headerSizeSlider_ == nullptr || subheaderSizeSlider_ == nullptr ||
        contentHeaderSizeSlider_ == nullptr || contentSubheaderSizeSlider_ == nullptr ||
        contentSizeSlider_ == nullptr) {
        return false;
    }

    outTheme.fonts.header = core::montserratFont(lv_slider_get_value(headerSizeSlider_));
    outTheme.fonts.subheader = core::montserratFont(lv_slider_get_value(subheaderSizeSlider_));
    outTheme.fonts.contentHeader = core::montserratFont(lv_slider_get_value(contentHeaderSizeSlider_));
    outTheme.fonts.contentSubheader = core::montserratFont(lv_slider_get_value(contentSubheaderSizeSlider_));
    outTheme.fonts.content = core::montserratFont(lv_slider_get_value(contentSizeSlider_));
    return true;
}

void ThemePreviewScreen::applyThemeToInputs(const core::UiTheme& theme)
{
    if (schemaNameField_) {
        lv_textarea_set_text(schemaNameField_, theme.name.c_str());
    }

    if (headerBgField_) lv_textarea_set_text(headerBgField_, colorToHexString(theme.colors.headerBg).c_str());
    if (headerFgField_) lv_textarea_set_text(headerFgField_, colorToHexString(theme.colors.headerFg).c_str());
    if (contentBgField_) lv_textarea_set_text(contentBgField_, colorToHexString(theme.colors.contentBg).c_str());
    if (contentFgField_) lv_textarea_set_text(contentFgField_, colorToHexString(theme.colors.contentFg).c_str());
    if (buttonBgField_) lv_textarea_set_text(buttonBgField_, colorToHexString(theme.colors.buttonBg).c_str());
    if (buttonFgField_) lv_textarea_set_text(buttonFgField_, colorToHexString(theme.colors.buttonFg).c_str());
    if (shadowField_) lv_textarea_set_text(shadowField_, colorToHexString(theme.colors.shadow).c_str());
    if (dangerBgField_) lv_textarea_set_text(dangerBgField_, colorToHexString(theme.colors.dangerBg).c_str());
    if (dangerFgField_) lv_textarea_set_text(dangerFgField_, colorToHexString(theme.colors.dangerFg).c_str());

    updateSingleSwatch(headerBgField_, headerBgSwatch_);
    updateSingleSwatch(headerFgField_, headerFgSwatch_);
    updateSingleSwatch(contentBgField_, contentBgSwatch_);
    updateSingleSwatch(contentFgField_, contentFgSwatch_);
    updateSingleSwatch(buttonBgField_, buttonBgSwatch_);
    updateSingleSwatch(buttonFgField_, buttonFgSwatch_);
    updateSingleSwatch(shadowField_, shadowSwatch_);
    updateSingleSwatch(dangerBgField_, dangerBgSwatch_);
    updateSingleSwatch(dangerFgField_, dangerFgSwatch_);

    if (headerSizeSlider_) {
        lv_slider_set_value(headerSizeSlider_, fontSizeFromPtr(theme.fonts.header), LV_ANIM_OFF);
        lv_event_send(headerSizeSlider_, LV_EVENT_VALUE_CHANGED, nullptr);
    }
    if (subheaderSizeSlider_) {
        lv_slider_set_value(subheaderSizeSlider_, fontSizeFromPtr(theme.fonts.subheader), LV_ANIM_OFF);
        lv_event_send(subheaderSizeSlider_, LV_EVENT_VALUE_CHANGED, nullptr);
    }
    if (contentHeaderSizeSlider_) {
        lv_slider_set_value(contentHeaderSizeSlider_, fontSizeFromPtr(theme.fonts.contentHeader), LV_ANIM_OFF);
        lv_event_send(contentHeaderSizeSlider_, LV_EVENT_VALUE_CHANGED, nullptr);
    }
    if (contentSubheaderSizeSlider_) {
        lv_slider_set_value(contentSubheaderSizeSlider_, fontSizeFromPtr(theme.fonts.contentSubheader), LV_ANIM_OFF);
        lv_event_send(contentSubheaderSizeSlider_, LV_EVENT_VALUE_CHANGED, nullptr);
    }
    if (contentSizeSlider_) {
        lv_slider_set_value(contentSizeSlider_, fontSizeFromPtr(theme.fonts.content), LV_ANIM_OFF);
        lv_event_send(contentSizeSlider_, LV_EVENT_VALUE_CHANGED, nullptr);
    }
}

} // namespace lightinator::ui::screens

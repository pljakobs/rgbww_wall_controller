#include "ui/screens/ThemeSelectorScreen.h"

namespace lightinator::ui::screens {

ThemeSelectorScreen::ThemeSelectorScreen(const core::UiTheme& theme)
    : theme_(theme)
{
}

void ThemeSelectorScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void ThemeSelectorScreen::setOnThemeListRequested(
    std::function<std::vector<core::UiTheme>()> callback)
{
    onThemeListRequested_ = std::move(callback);
}

void ThemeSelectorScreen::setOnThemeApplyRequested(
    std::function<void(const core::UiTheme&)> callback)
{
    onThemeApplyRequested_ = std::move(callback);
}

void ThemeSelectorScreen::setOnThemeEditRequested(
    std::function<void(const core::UiTheme&, const String&)> callback)
{
    onThemeEditRequested_ = std::move(callback);
}

// ---------------------------------------------------------------------------
// mount
// ---------------------------------------------------------------------------

void ThemeSelectorScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text      = "Themes";
    header.showClose = true;
    header.height    = theme_.headerHeight;
    header.color     = theme_.colors.headerBg;
    header.font      = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
    decorated_->setOnCloseRequested([this]() {
        if (onCloseRequested_) onCloseRequested_();
    });
    decorated_->mount(parent);
    setRoot(decorated_->root());

    lv_obj_t* body = decorated_->bodySlot();

    // --- root layout ---
    lv_obj_t* layout = lv_obj_create(body);
    lv_obj_set_size(layout, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(layout, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(layout, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(layout, 0, 0);
    lv_obj_set_style_pad_all(layout, 12, 0);
    lv_obj_set_style_pad_row(layout, 12, 0);
    lv_obj_set_flex_flow(layout, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(layout, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);

    // --- dropdown row ---
    lv_obj_t* dropRow = lv_obj_create(layout);
    lv_obj_set_size(dropRow, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(dropRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(dropRow, 0, 0);
    lv_obj_set_style_pad_all(dropRow, 0, 0);
    lv_obj_set_style_pad_column(dropRow, 8, 0);
    lv_obj_set_flex_flow(dropRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dropRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t* dropLabel = lv_label_create(dropRow);
    lv_label_set_text_static(dropLabel, "Select theme:");
    lv_obj_set_style_text_font(dropLabel, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(dropLabel, theme_.colors.contentFg, 0);
    lv_obj_set_width(dropLabel, 140);

    schemeDropdown_ = lv_dropdown_create(dropRow);
    lv_obj_set_height(schemeDropdown_, 52);
    lv_obj_set_flex_grow(schemeDropdown_, 1);
    lv_obj_set_style_text_font(schemeDropdown_, theme_.fonts.contentSubheader, LV_PART_MAIN);
    lv_obj_set_style_text_font(schemeDropdown_, theme_.fonts.contentSubheader, LV_PART_SELECTED);
    lv_obj_set_style_text_color(schemeDropdown_, theme_.colors.contentFg, 0);
    lv_obj_set_style_bg_color(schemeDropdown_, theme_.colors.contentBg, 0);
    lv_obj_set_style_border_color(schemeDropdown_, theme_.colors.shadow, 0);
    lv_dropdown_set_dir(schemeDropdown_, LV_DIR_BOTTOM);
    lv_obj_set_style_pad_ver(schemeDropdown_, 10, LV_PART_MAIN);

    if (onThemeListRequested_) {
        availableThemes_ = onThemeListRequested_();
    }

    String options;
    int selectedIndex = 0;
    for (size_t i = 0; i < availableThemes_.size(); ++i) {
        if (options.length() != 0) options += "\n";
        options += availableThemes_[i].name;
        if ((theme_.id.length() != 0 && availableThemes_[i].id == theme_.id) ||
            availableThemes_[i].name == theme_.name) {
            selectedIndex = static_cast<int>(i);
        }
    }
    lv_dropdown_set_options(schemeDropdown_, options.c_str());
    lv_dropdown_set_selected(schemeDropdown_, static_cast<uint16_t>(selectedIndex));
    lv_obj_add_event_cb(schemeDropdown_, onDropdownChanged, LV_EVENT_VALUE_CHANGED, this);

    if (lv_obj_t* list = lv_dropdown_get_list(schemeDropdown_)) {
        lv_obj_set_style_max_height(list, 260, 0);
        lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_ACTIVE);
        lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
        lv_obj_set_style_text_font(list, theme_.fonts.contentSubheader, 0);
        lv_obj_set_style_pad_row(list, 8, 0);
        lv_obj_set_style_bg_color(list, theme_.colors.contentBg, 0);
        lv_obj_set_style_text_color(list, theme_.colors.contentFg, 0);
    }

    // --- color swatch preview ---
    lv_obj_t* swatchRow = lv_obj_create(layout);
    lv_obj_set_size(swatchRow, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(swatchRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(swatchRow, 0, 0);
    lv_obj_set_style_pad_all(swatchRow, 0, 0);
    lv_obj_set_style_pad_column(swatchRow, 6, 0);
    lv_obj_set_flex_flow(swatchRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(swatchRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    static const char* kSwatchLabels[kSwatchCount] = {
        "hBg", "hFg", "cBg", "cFg", "bBg", "bFg"
    };

    for (int i = 0; i < kSwatchCount; ++i) {
        lv_obj_t* col = lv_obj_create(swatchRow);
        lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(col, 0, 0);
        lv_obj_set_style_pad_all(col, 0, 0);
        lv_obj_set_style_pad_row(col, 2, 0);
        lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(col, 1);
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER);

        swatches_[i] = lv_obj_create(col);
        lv_obj_set_size(swatches_[i], 40, 40);
        lv_obj_set_style_radius(swatches_[i], 4, 0);
        lv_obj_set_style_bg_opa(swatches_[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(swatches_[i], 1, 0);
        lv_obj_set_style_border_color(swatches_[i], theme_.colors.shadow, 0);

        lv_obj_t* lbl = lv_label_create(col);
        lv_label_set_text_static(lbl, kSwatchLabels[i]);
        lv_obj_set_style_text_font(lbl, theme_.fonts.content, 0);
        lv_obj_set_style_text_color(lbl, theme_.colors.contentFg, 0);
    }

    // Populate swatches with the current selection
    if (!availableThemes_.empty()) {
        updateSwatches(availableThemes_[static_cast<size_t>(selectedIndex)]);
    }

    // --- action buttons ---
    lv_obj_t* btnRow = lv_obj_create(layout);
    lv_obj_set_size(btnRow, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(btnRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btnRow, 0, 0);
    lv_obj_set_style_pad_all(btnRow, 0, 0);
    lv_obj_set_style_pad_column(btnRow, 12, 0);
    lv_obj_set_flex_flow(btnRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btnRow, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t* applyBtn = lv_btn_create(btnRow);
    lv_obj_set_size(applyBtn, 138, 48);
    lv_obj_set_style_bg_color(applyBtn, theme_.colors.buttonBg, 0);
    lv_obj_set_style_border_width(applyBtn, 0, 0);
    lv_obj_add_event_cb(applyBtn, onApplyButtonEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* applyLabel = lv_label_create(applyBtn);
    lv_label_set_text_static(applyLabel, "Apply");
    lv_obj_set_style_text_font(applyLabel, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(applyLabel, theme_.colors.buttonFg, 0);
    lv_obj_center(applyLabel);

    lv_obj_t* editBtn = lv_btn_create(btnRow);
    lv_obj_set_size(editBtn, 138, 48);
    lv_obj_set_style_bg_color(editBtn, theme_.colors.buttonBg, 0);
    lv_obj_set_style_border_width(editBtn, 0, 0);
    lv_obj_add_event_cb(editBtn, onEditButtonEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* editLabel = lv_label_create(editBtn);
    lv_label_set_text_static(editLabel, "Edit");
    lv_obj_set_style_text_font(editLabel, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(editLabel, theme_.colors.buttonFg, 0);
    lv_obj_center(editLabel);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void ThemeSelectorScreen::updateSwatches(const core::UiTheme& sel)
{
    const lv_color_t colors[kSwatchCount] = {
        sel.colors.headerBg,
        sel.colors.headerFg,
        sel.colors.contentBg,
        sel.colors.contentFg,
        sel.colors.buttonBg,
        sel.colors.buttonFg,
    };
    for (int i = 0; i < kSwatchCount; ++i) {
        if (swatches_[i]) {
            lv_obj_set_style_bg_color(swatches_[i], colors[i], 0);
        }
    }
}

String ThemeSelectorScreen::computeSuggestedName(const core::UiTheme& theme) const
{
    // Choose the base suffix depending on origin.
    String base = theme.isBuiltin ? (theme.name + "-user") : (theme.name + "-edit");

    // Find the first unused name: base, base-1, base-2, …
    auto nameTaken = [&](const String& n) {
        for (const auto& t : availableThemes_) {
            if (t.name == n) return true;
        }
        return false;
    };

    if (!nameTaken(base)) return base;

    for (int i = 1; i < 100; ++i) {
        String candidate = base + "-" + String(i);
        if (!nameTaken(candidate)) return candidate;
    }
    return base;
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------

void ThemeSelectorScreen::onDropdownChanged(lv_event_t* event)
{
    auto* self = static_cast<ThemeSelectorScreen*>(lv_event_get_user_data(event));
    if (self == nullptr || self->schemeDropdown_ == nullptr) return;

    const uint16_t idx = lv_dropdown_get_selected(self->schemeDropdown_);
    if (idx < self->availableThemes_.size()) {
        self->updateSwatches(self->availableThemes_[idx]);
    }
}

void ThemeSelectorScreen::onApplyButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<ThemeSelectorScreen*>(lv_event_get_user_data(event));
    if (self == nullptr || self->schemeDropdown_ == nullptr) return;

    const uint16_t idx = lv_dropdown_get_selected(self->schemeDropdown_);
    if (idx < self->availableThemes_.size() && self->onThemeApplyRequested_) {
        core::UiTheme selected = self->availableThemes_[idx];
        self->onThemeApplyRequested_(selected);
    }
}

void ThemeSelectorScreen::onEditButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<ThemeSelectorScreen*>(lv_event_get_user_data(event));
    if (self == nullptr || self->schemeDropdown_ == nullptr) return;

    const uint16_t idx = lv_dropdown_get_selected(self->schemeDropdown_);
    if (idx < self->availableThemes_.size() && self->onThemeEditRequested_) {
        core::UiTheme selected = self->availableThemes_[idx];
        self->onThemeEditRequested_(selected, self->computeSuggestedName(selected));
    }
}

} // namespace lightinator::ui::screens

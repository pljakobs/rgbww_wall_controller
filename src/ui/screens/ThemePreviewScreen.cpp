#include "ui/screens/ThemePreviewScreen.h"

namespace lightinator::ui::screens {

ThemePreviewScreen::ThemePreviewScreen(const core::UiTheme& theme)
    : theme_(theme)
{
}

void ThemePreviewScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void ThemePreviewScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text      = "Theme Preview";
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

    // Tabview filling the entire body slot
    lv_obj_t* tabview = lv_tabview_create(body, LV_DIR_TOP, 48);
    lv_obj_set_size(tabview, lv_pct(100), lv_pct(100));
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
}

void ThemePreviewScreen::buildColorsTab(lv_obj_t* tab)
{
    lv_obj_set_style_bg_color(tab, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(tab, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(tab, 8, 0);
    lv_obj_set_style_pad_row(tab, 6, 0);
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    addColorRow(tab, "headerBg",   theme_.colors.headerBg);
    addColorRow(tab, "headerFg",   theme_.colors.headerFg);
    addColorRow(tab, "contentBg",  theme_.colors.contentBg);
    addColorRow(tab, "contentFg",  theme_.colors.contentFg);
    addColorRow(tab, "buttonBg",   theme_.colors.buttonBg);
    addColorRow(tab, "buttonFg",   theme_.colors.buttonFg);
    addColorRow(tab, "shadow",     theme_.colors.shadow);
    addColorRow(tab, "dangerBg",   theme_.colors.dangerBg);
    addColorRow(tab, "dangerFg",   theme_.colors.dangerFg);
}

void ThemePreviewScreen::buildFontsTab(lv_obj_t* tab)
{
    lv_obj_set_style_bg_color(tab, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(tab, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(tab, 8, 0);
    lv_obj_set_style_pad_row(tab, 10, 0);
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    addFontRow(tab, "header",           theme_.fonts.header);
    addFontRow(tab, "subheader",        theme_.fonts.subheader);
    addFontRow(tab, "contentHeader",    theme_.fonts.contentHeader);
    addFontRow(tab, "contentSubheader", theme_.fonts.contentSubheader);
    addFontRow(tab, "content",          theme_.fonts.content);
}

void ThemePreviewScreen::addColorRow(lv_obj_t* parent, const char* name, lv_color_t color)
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

    // Color swatch
    lv_obj_t* swatch = lv_obj_create(row);
    lv_obj_set_size(swatch, 36, 36);
    lv_obj_set_style_radius(swatch, 4, 0);
    lv_obj_set_style_bg_color(swatch, color, 0);
    lv_obj_set_style_bg_opa(swatch, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(swatch, 1, 0);
    lv_obj_set_style_border_color(swatch, theme_.colors.shadow, 0);

    // Label
    lv_obj_t* label = lv_label_create(row);
    lv_label_set_text_static(label, name);
    lv_obj_set_style_text_font(label, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(label, theme_.colors.contentFg, 0);
}

void ThemePreviewScreen::addFontRow(lv_obj_t* parent, const char* name, const lv_font_t* font)
{
    // Row container
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(row, 2, 0);

    // Small font name label (using content font)
    lv_obj_t* nameLabel = lv_label_create(row);
    lv_label_set_text_static(nameLabel, name);
    lv_obj_set_style_text_font(nameLabel, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(nameLabel, theme_.colors.contentFg, LV_STATE_DEFAULT);
    lv_obj_set_style_opa(nameLabel, LV_OPA_60, 0);

    // Sample text rendered in the actual font
    lv_obj_t* sampleLabel = lv_label_create(row);
    lv_label_set_text_static(sampleLabel, "Aa Bb 012");
    lv_obj_set_style_text_font(sampleLabel, font, 0);
    lv_obj_set_style_text_color(sampleLabel, theme_.colors.contentFg, 0);
}

} // namespace lightinator::ui::screens

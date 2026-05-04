#include "ui/screens/MainScreen.h"

namespace lightinator::ui::screens {

MainScreen::MainScreen(core::HsvColor color, const core::UiTheme& theme)
    : color_(core::clampHsv(color)), theme_(theme)
{
}

void MainScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text = "Lightinator";
    header.showClose = false;
    header.showBurger = true;
    header.height = theme_.headerHeight;
    header.color  = theme_.colors.headerBg;
    header.font   = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
    decorated_->setOnBurgerTapped([this]() { showBurgerMenu(); });
    decorated_->mount(parent);
    decorated_->setHeaderStatusIcon(LV_SYMBOL_WIFI, wifiConnected_);
    decorated_->setOnHeaderStatusIconTapped([this]() {
        if(onOpenNetworkInfoRequested_) {
            onOpenNetworkInfoRequested_();
        }
    });
    setRoot(decorated_->root());

    bodyLayout_ = std::make_unique<lvgl::widget::Object>(decorated_->bodySlot());
    bodyLayout_->SetSize(lv_pct(100), lv_pct(100))
        ->SetStyleBgOpa(LV_OPA_TRANSP, 0)
        ->SetStyleBorderWidth(0, 0)
        ->SetStylePadAll(0, 0)
        ->SetStylePadRow(18, 0)
        ->SetFlexFlow(LV_FLEX_FLOW_COLUMN)
        ->SetFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    previewSwatch_ = std::make_unique<lvgl::widget::Object>(bodyLayout_->GetObj());
    previewSwatch_->SetSize(210, 96)
        ->SetStyleRadius(8, 0)
        ->SetStyleBorderWidth(1, 0)
        ->SetStyleBorderColor(theme_.colors.shadow, 0);

    previewLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    previewLabel_->SetStyleTextFont(theme_.fonts.subheader, 0)
        ->SetStyleTextColor(theme_.colors.contentFg, 0);

    openButton_ = std::make_unique<lvgl::widget::Button>(bodyLayout_->GetObj());
    openButton_->SetSize(240, 58)
        ->SetStyleRadius(10, 0)
        ->SetStyleBgColor(theme_.colors.buttonBg, 0)
        ->SetStyleBorderWidth(0, 0)
        ->AddEventCbClicked(onOpenButtonEvent, this);
    lv_obj_t* button = openButton_->GetObj();

    auto* buttonLabel = lv_label_create(button);
    lv_label_set_text_static(buttonLabel, "Open color picker");
    lv_obj_set_style_text_font(buttonLabel, theme_.fonts.subheader, 0);
    lv_obj_set_style_text_color(buttonLabel, theme_.colors.buttonFg, 0);
    lv_obj_center(buttonLabel);

    setColorPreview(color_);
}

void MainScreen::setColorPreview(core::HsvColor color)
{
    color_ = core::clampHsv(color);
    if (!previewSwatch_ || !previewLabel_) {
        return;
    }

    lv_color_t rgb = lv_color_hsv_to_rgb(
        core::toLvHue(color_.h), core::toLvPercent(color_.s), core::toLvPercent(color_.v));
    previewSwatch_->SetStyleBgColor(rgb, 0);

    const uint32_t hTenths = ((color_.h * 10U) + (core::kHueScale / 2U)) / core::kHueScale;
    const uint32_t sTenths = ((color_.s * 10U) + (core::kPercentScale / 2U)) / core::kPercentScale;
    const uint32_t vTenths = ((color_.v * 10U) + (core::kPercentScale / 2U)) / core::kPercentScale;

    lv_label_set_text_fmt(
        previewLabel_->GetObj(),
        "H:%lu.%lu deg  S:%lu.%lu%%  V:%lu.%lu%%",
        static_cast<unsigned long>(hTenths / 10U),
        static_cast<unsigned long>(hTenths % 10U),
        static_cast<unsigned long>(sTenths / 10U),
        static_cast<unsigned long>(sTenths % 10U),
        static_cast<unsigned long>(vTenths / 10U),
        static_cast<unsigned long>(vTenths % 10U));
}

void MainScreen::setOnOpenColorPickerRequested(std::function<void()> callback)
{
    onOpenColorPickerRequested_ = std::move(callback);
}

void MainScreen::setOnOpenNetworkInfoRequested(std::function<void()> callback)
{
    onOpenNetworkInfoRequested_ = std::move(callback);
}

void MainScreen::setOnOpenThemePreviewRequested(std::function<void()> callback)
{
    onOpenThemePreviewRequested_ = std::move(callback);
}

void MainScreen::setWifiConnected(bool connected)
{
    wifiConnected_ = connected;
    if(decorated_) {
        decorated_->setHeaderStatusIcon(LV_SYMBOL_WIFI, wifiConnected_);
    }
}

void MainScreen::onOpenButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    if (self->onOpenColorPickerRequested_) {
        self->onOpenColorPickerRequested_();
    }
}

void MainScreen::onMenuItemEvent(lv_event_t* event)
{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }
    self->hideBurgerMenu();
    if (self->onOpenThemePreviewRequested_) {
        self->onOpenThemePreviewRequested_();
    }
}

void MainScreen::onMenuDismissEvent(lv_event_t* event)
{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }
    self->hideBurgerMenu();
}

void MainScreen::showBurgerMenu()
{
    if (burgerMenuPanel_) {
        hideBurgerMenu();
        return;
    }

    lv_obj_t* root = decorated_->root();

    // Transparent full-screen dismiss layer
    lv_obj_t* dismiss = lv_obj_create(root);
    lv_obj_set_size(dismiss, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(dismiss, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(dismiss, 0, 0);
    lv_obj_set_style_pad_all(dismiss, 0, 0);
    lv_obj_add_flag(dismiss, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(dismiss, onMenuDismissEvent, LV_EVENT_CLICKED, this);

    // Menu panel
    lv_obj_t* panel = lv_obj_create(dismiss);
    burgerMenuPanel_ = panel;
    lv_obj_set_size(panel, 200, LV_SIZE_CONTENT);
    lv_obj_set_pos(panel, 14, theme_.headerHeight + 20);
    lv_obj_set_style_bg_color(panel, theme_.colors.buttonBg, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(panel, 10, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, theme_.colors.shadow, 0);
    lv_obj_set_style_pad_all(panel, 6, 0);
    lv_obj_set_style_pad_row(panel, 4, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    // "Theme View" item
    lv_obj_t* item = lv_obj_create(panel);
    lv_obj_set_width(item, lv_pct(100));
    lv_obj_set_height(item, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_set_style_pad_all(item, 10, 0);
    lv_obj_set_style_radius(item, 8, 0);
    lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(item, theme_.colors.headerBg, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(item, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_add_event_cb(item, onMenuItemEvent, LV_EVENT_CLICKED, this);

    lv_obj_t* label = lv_label_create(item);
    lv_label_set_text_static(label, "Theme View");
    lv_obj_set_style_text_font(label, theme_.fonts.subheader, 0);
    lv_obj_set_style_text_color(label, theme_.colors.buttonFg, 0);
}

void MainScreen::hideBurgerMenu()
{
    if (burgerMenuPanel_) {
        // delete the dismiss layer (parent of panel)
        lv_obj_del(lv_obj_get_parent(burgerMenuPanel_));
        burgerMenuPanel_ = nullptr;
    }
}

} // namespace lightinator::ui::screens

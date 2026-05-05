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

void MainScreen::setOnOpenSettingsRequested(std::function<void()> callback)
{
    onOpenSettingsRequested_ = std::move(callback);
}

void MainScreen::setOnOpenMenuTestRequested(std::function<void()> callback)
{
    onOpenMenuTestRequested_ = std::move(callback);
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

void MainScreen::onMenuThemeEvent(lv_event_t* event)
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

void MainScreen::onMenuColorPickerEvent(lv_event_t* event)
{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }
    self->hideBurgerMenu();
    if (self->onOpenColorPickerRequested_) {
        self->onOpenColorPickerRequested_();
    }
}

void MainScreen::onMenuNetworkInfoEvent(lv_event_t* event)
{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }
    self->hideBurgerMenu();
    if (self->onOpenNetworkInfoRequested_) {
        self->onOpenNetworkInfoRequested_();
    }
}

void MainScreen::onMenuSettingsEvent(lv_event_t* event)
{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }
    self->hideBurgerMenu();
    if (self->onOpenSettingsRequested_) {
        self->onOpenSettingsRequested_();
    }
}

void MainScreen::onMenuMenuTestEvent(lv_event_t* event)
{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }
    self->hideBurgerMenu();
    if (self->onOpenMenuTestRequested_) {
        self->onOpenMenuTestRequested_();
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
    bool createdNow = false;
    if (!burgerMenuOverlay_.valid()) {
        createdNow = true;
        lv_obj_t* root = decorated_->root();

        // Full-screen transparent dismiss layer; drawer itself provides visual overlay.
        lv_obj_t* overlay = lv_obj_create(root);
        burgerMenuOverlay_.attach(overlay);
        lv_obj_add_flag(overlay, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(overlay, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_pos(overlay, 0, 0);
        lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_opa(overlay, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(overlay, 0, 0);
        lv_obj_set_style_pad_all(overlay, 0, 0);
        lv_obj_add_event_cb(overlay, onMenuDismissEvent, LV_EVENT_CLICKED, this);

        // Left drawer panel occupying half the screen below the header.
        lv_obj_t* panel = lv_obj_create(overlay);
        burgerMenuPanel_.attach(panel);
        lv_obj_add_flag(panel, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(panel, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(panel, theme_.colors.buttonBg, 0);
        lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(panel, 0, 0);
        lv_obj_set_style_border_width(panel, 1, 0);
        lv_obj_set_style_border_color(panel, theme_.colors.shadow, 0);
        lv_obj_set_style_pad_all(panel, 0, 0);
        lv_obj_set_style_pad_row(panel, 0, 0);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

        lv_obj_t* title = lv_label_create(panel);
        lv_label_set_text_static(title, "Menu");
        lv_obj_set_style_text_font(title, theme_.fonts.subheader, 0);
        lv_obj_set_style_text_color(title, theme_.colors.buttonFg, 0);
        lv_obj_set_style_pad_all(title, 12, 0);

        lv_obj_t* list = lv_list_create(panel);
        lv_obj_set_size(list, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(list, 0, 0);
        lv_obj_set_style_pad_all(list, 0, 0);
        lv_obj_set_style_pad_row(list, 0, 0);
        lv_obj_set_scroll_dir(list, LV_DIR_VER);
        lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);

        auto styleListButton = [this](lv_obj_t* btn) {
            lv_obj_set_width(btn, lv_pct(100));
            lv_obj_set_style_border_width(btn, 0, 0);
            lv_obj_set_style_radius(btn, 0, 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_color(btn, theme_.colors.headerBg, LV_STATE_PRESSED);
            lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_STATE_PRESSED);
            lv_obj_set_style_text_font(btn, theme_.fonts.contentHeader, 0);
            lv_obj_set_style_text_color(btn, theme_.colors.buttonFg, 0);
            lv_obj_set_style_pad_left(btn, 16, 0);
            lv_obj_set_style_pad_right(btn, 16, 0);
            lv_obj_set_height(btn, 64);
            lv_obj_set_ext_click_area(btn, 8);
        };

        lv_obj_t* colorItem = lv_list_add_btn(list, nullptr, "Color Picker");
        styleListButton(colorItem);
        lv_obj_add_event_cb(colorItem, onMenuColorPickerEvent, LV_EVENT_CLICKED, this);

        lv_obj_t* networkItem = lv_list_add_btn(list, nullptr, "Network Info");
        styleListButton(networkItem);
        lv_obj_add_event_cb(networkItem, onMenuNetworkInfoEvent, LV_EVENT_CLICKED, this);

        lv_obj_t* themeItem = lv_list_add_btn(list, nullptr, "Theme View");
        styleListButton(themeItem);
        lv_obj_add_event_cb(themeItem, onMenuThemeEvent, LV_EVENT_CLICKED, this);

    // ui-scaffold:main-left:Settings:menu-item
    lv_obj_t* settingsItem = lv_list_add_btn(list, nullptr, "Settings");
    styleListButton(settingsItem);
    lv_obj_add_event_cb(settingsItem, onMenuSettingsEvent, LV_EVENT_CLICKED, this);

        // ui-scaffold:main-left:MenuTest:menu-item
        lv_obj_t* menuTestItem = lv_list_add_btn(list, nullptr, "MenuTest");
        styleListButton(menuTestItem);
        lv_obj_add_event_cb(menuTestItem, onMenuMenuTestEvent, LV_EVENT_CLICKED, this);
    }

    lv_obj_t* overlay = burgerMenuOverlay_.get();
    if (overlay == nullptr) {
        return;
    }

    if (createdNow || lv_obj_has_flag(overlay, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(overlay, LV_OBJ_FLAG_HIDDEN);
    } else {
        hideBurgerMenu();
        return;
    }

    updateBurgerMenuGeometry();
}

void MainScreen::hideBurgerMenu()
{
    if (lv_obj_t* overlay = burgerMenuOverlay_.get()) {
        lv_obj_add_flag(overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

void MainScreen::updateBurgerMenuGeometry()
{
    lv_obj_t* overlay = burgerMenuOverlay_.get();
    lv_obj_t* panel = burgerMenuPanel_.get();
    if (overlay == nullptr || panel == nullptr || decorated_ == nullptr) {
        return;
    }

    lv_obj_t* root = decorated_->root();
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_size(overlay, lv_obj_get_width(root), lv_obj_get_height(root));

    const lv_coord_t rootPadTop = lv_obj_get_style_pad_top(root, LV_PART_MAIN);
    const lv_coord_t top = rootPadTop + theme_.headerHeight;
    lv_coord_t drawerWidth = (lv_obj_get_width(root) * 50) / 100;
    if (drawerWidth < 180) {
        drawerWidth = 180;
    }
    lv_coord_t drawerHeight = lv_obj_get_height(root) - top;
    if (drawerHeight < 80) {
        drawerHeight = 80;
    }

    lv_obj_set_size(panel, drawerWidth, drawerHeight);
    lv_obj_set_pos(panel, 0, top);
}

} // namespace lightinator::ui::screens

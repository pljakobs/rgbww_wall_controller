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
    header.height = theme_.headerHeight;
    header.color  = theme_.colors.headerBg;
    header.font   = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
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
        ->SetStyleBorderColor(lv_color_hex(0x202020), 0);

    previewLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    previewLabel_->SetStyleTextFont(&lv_font_montserrat_24, 0)
        ->SetStyleTextColor(lv_color_hex(0xD8DEE9), 0);

    openButton_ = std::make_unique<lvgl::widget::Button>(bodyLayout_->GetObj());
    openButton_->SetSize(240, 58)
        ->SetStyleRadius(10, 0)
        ->SetStyleBgColor(lv_color_hex(0x1D5A8E), 0)
        ->SetStyleBorderWidth(0, 0)
        ->AddEventCbClicked(onOpenButtonEvent, this);
    lv_obj_t* button = openButton_->GetObj();

    auto* buttonLabel = lv_label_create(button);
    lv_label_set_text_static(buttonLabel, "Open color picker");
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

} // namespace lightinator::ui::screens

#include "ui/screens/NetworkInfoScreen.h"

namespace lightinator::ui::screens {

NetworkInfoScreen::NetworkInfoScreen(const core::UiTheme& theme) : theme_(theme)
{
}

void NetworkInfoScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text = "Network Info";
    header.showClose = true;
    header.height = theme_.headerHeight;
    header.color  = theme_.colors.headerBg;
    header.font   = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
    decorated_->setOnCloseRequested([this]() {
        if(onCloseRequested_) {
            onCloseRequested_();
        }
    });
    decorated_->mount(parent);
    setRoot(decorated_->root());

    bodyLayout_ = std::make_unique<lvgl::widget::Object>(decorated_->bodySlot());
    bodyLayout_->SetSize(lv_pct(100), lv_pct(100))
        ->SetStyleBgOpa(LV_OPA_TRANSP, 0)
        ->SetStyleBorderWidth(0, 0)
        ->SetStylePadAll(0, 0)
        ->SetStylePadRow(14, 0)
        ->SetFlexFlow(LV_FLEX_FLOW_COLUMN)
        ->SetFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    connectedLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    ipLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    netmaskLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    gatewayLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    neighboursTitleLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    neighboursList_ = std::make_unique<lvgl::widget::Object>(bodyLayout_->GetObj());

    const lv_color_t textColor = lv_color_hex(0xD8DEE9);
    connectedLabel_->SetStyleTextFont(&lv_font_montserrat_24, 0)->SetStyleTextColor(textColor, 0);
    ipLabel_->SetStyleTextFont(&lv_font_montserrat_24, 0)->SetStyleTextColor(textColor, 0);
    netmaskLabel_->SetStyleTextFont(&lv_font_montserrat_24, 0)->SetStyleTextColor(textColor, 0);
    gatewayLabel_->SetStyleTextFont(&lv_font_montserrat_24, 0)->SetStyleTextColor(textColor, 0);
    neighboursTitleLabel_->SetStyleTextFont(&lv_font_montserrat_24, 0)->SetStyleTextColor(textColor, 0);

    neighboursList_->SetWidth(lv_pct(100))
        ->SetStyleBgColor(lv_color_hex(0x1A2129), 0)
        ->SetStyleBgOpa(LV_OPA_COVER, 0)
        ->SetStyleBorderWidth(1, 0)
        ->SetStyleBorderColor(lv_color_hex(0x334455), 0)
        ->SetStyleRadius(10, 0)
        ->SetStylePadAll(10, 0)
        ->SetStylePadRow(8, 0)
        ->SetFlexFlow(LV_FLEX_FLOW_COLUMN)
        ->SetFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_height(neighboursList_->GetObj(), 190);
    lv_obj_set_scroll_dir(neighboursList_->GetObj(), LV_DIR_VER);

    refreshLabels();
    refreshNeighbours();
}

void NetworkInfoScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void NetworkInfoScreen::setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway)
{
    connected_ = connected;
    ip_ = ip;
    netmask_ = netmask;
    gateway_ = gateway;
    refreshLabels();
}

void NetworkInfoScreen::setNeighbours(const std::vector<Neighbour>& neighbours)
{
    neighbours_ = neighbours;
    refreshNeighbours();
}

void NetworkInfoScreen::refreshLabels()
{
    if(!connectedLabel_ || !ipLabel_ || !netmaskLabel_ || !gatewayLabel_) {
        return;
    }

    lv_label_set_text_fmt(connectedLabel_->GetObj(), "WiFi: %s", connected_ ? "connected" : "disconnected");
    lv_label_set_text_fmt(ipLabel_->GetObj(), "IP address: %s", ip_.length() > 0 ? ip_.c_str() : "-");
    lv_label_set_text_fmt(netmaskLabel_->GetObj(), "Netmask: %s", netmask_.length() > 0 ? netmask_.c_str() : "-");
    lv_label_set_text_fmt(gatewayLabel_->GetObj(), "Next hop: %s", gateway_.length() > 0 ? gateway_.c_str() : "-");
    lv_label_set_text_static(neighboursTitleLabel_->GetObj(), "Neighbours:");
}

void NetworkInfoScreen::refreshNeighbours()
{
    if(!neighboursList_) {
        return;
    }

    lv_obj_clean(neighboursList_->GetObj());

    if(neighbours_.empty()) {
        lv_obj_t* row = lv_label_create(neighboursList_->GetObj());
        lv_obj_set_style_text_font(row, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(row, lv_color_hex(0xA9B4C2), 0);
        lv_label_set_text_static(row, "No visible neighbours");
        return;
    }

    for(const auto& neighbour : neighbours_) {
        lv_obj_t* row = lv_label_create(neighboursList_->GetObj());
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_style_text_font(row, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(row, lv_color_hex(0xD8DEE9), 0);

        const String name = neighbour.name.length() > 0 ? neighbour.name : String("unnamed");
        const String ip = neighbour.ip.length() > 0 ? neighbour.ip : String("-");
        const String text = name + "  (" + ip + ")";
        lv_label_set_text(row, text.c_str());
    }
}

} // namespace lightinator::ui::screens

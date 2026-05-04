#include "ui/AppUi.h"

#include <memory>

namespace lightinator::ui {

AppUi::~AppUi() = default;

bool AppUi::init()
{
    if (initialized_) {
        return true;
    }

    screen_ = lv_scr_act();
    if (screen_ == nullptr) {
        return false;
    }

    lv_obj_set_style_bg_color(screen_, lv_color_hex(0x0D1116), 0);
    lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, 0);

    root_ = lv_obj_create(screen_);
    lv_obj_remove_style_all(root_);
    lv_obj_set_size(root_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(root_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(root_, 0, 0);

    currentColor_ = core::clampHsv(currentColor_);
    navigator_ = std::make_unique<AppNavigator>(root_, state_, currentColor_);
    navigator_->showMainScreen();
    initialized_ = true;
    return true;
}

void AppUi::tickAnimation()
{
    // The app is event-driven; timer ticks are currently unused.
}

void AppUi::showWifiConfigScreen()
{
    navigator_->showWifiConfigScreen();
}

void AppUi::closeWifiConfigScreen()
{
    navigator_->closeWifiConfigScreen();
}

screens::WifiConfigScreen* AppUi::wifiConfigScreen()
{
    return navigator_->wifiConfigScreen();
}

void AppUi::setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway)
{
    const bool changed = state_.setNetworkInfo(connected, ip, netmask, gateway);
    if (!changed) {
        return;
    }
    // Visible-screen gating: only push to whichever screen is currently active.
    if (navigator_->mainScreen()) {
        navigator_->mainScreen()->setWifiConnected(state_.wifiConnected());
    }
    if (navigator_->networkInfoScreen()) {
        navigator_->networkInfoScreen()->setNetworkInfo(state_.wifiConnected(), state_.ipAddress(), state_.netmask(), state_.gateway());
    }
}

void AppUi::setNeighbours(const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours)
{
    const bool changed = state_.setNeighbours(neighbours);
    if (!changed) {
        return;
    }
    // Only update the neighbours list if NetworkInfoScreen is visible.
    if (navigator_->networkInfoScreen()) {
        navigator_->networkInfoScreen()->setNeighbours(state_.neighbours());
    }
}

} // namespace lightinator::ui

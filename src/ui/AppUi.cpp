#include "ui/AppUi.h"

#include <memory>

namespace lightinator::ui {

AppUi::~AppUi() = default;

AppUi::AppUi()
    : networkInfoPresenter_(state_)
{
}

bool AppUi::init()
{
    if (initialized_) {
        return true;
    }

    screen_ = lv_scr_act();
    if (screen_ == nullptr) {
        return false;
    }

    lv_obj_set_style_bg_color(screen_, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, 0);

    root_ = lv_obj_create(screen_);
    lv_obj_remove_style_all(root_);
    lv_obj_set_size(root_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(root_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(root_, 0, 0);

    currentColor_ = core::clampHsv(currentColor_);
    navigator_ = std::make_unique<AppNavigator>(root_, state_, currentColor_, theme_);
    navigator_->setOnNetworkInfoScreenChanged(
        [this](screens::NetworkInfoScreen* screen) {
            networkInfoPresenter_.bind(screen);
        });
    navigator_->showMainScreen();
    initialized_ = true;
    return true;
}

void AppUi::tickAnimation()
{
    // The app is event-driven; timer ticks are currently unused.
}

void AppUi::setTheme(const core::UiTheme& theme)
{
    theme_ = theme;
    if (screen_ != nullptr) {
        lv_obj_set_style_bg_color(screen_, theme_.colors.contentBg, 0);
    }
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
    // Update the wifi icon on main screen if visible.
    if (navigator_->mainScreen()) {
        navigator_->mainScreen()->setWifiConnected(state_.wifiConnected());
    }
    // Delegate network info rendering to the presenter.
    networkInfoPresenter_.onNetworkInfoChanged();
}

void AppUi::setNeighbours(const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours)
{
    const bool changed = state_.setNeighbours(neighbours);
    if (!changed) {
        return;
    }
    // Delegate neighbour list rendering to the presenter.
    networkInfoPresenter_.onNeighboursChanged();
}

} // namespace lightinator::ui

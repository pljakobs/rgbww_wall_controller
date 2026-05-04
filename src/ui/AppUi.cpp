#include "ui/AppUi.h"

#include <memory>

#include "ui/screens/ColorPickerScreen.h"
#include "ui/screens/MainScreen.h"
#include "ui/screens/NetworkInfoScreen.h"
#include "ui/screens/WifiConfigScreen.h"

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
    showMainScreen();
    initialized_ = true;
    return true;
}

void AppUi::tickAnimation()
{
    // The demo app is event-driven; timer ticks are currently unused.
}

void AppUi::clearRoot()
{
    if (root_ == nullptr) {
        return;
    }

    lv_obj_clean(root_);
}

void AppUi::pushStateToActiveScreen()
{
    if (mainScreen_) {
        mainScreen_->setWifiConnected(state_.wifiConnected());
    }
    if (networkInfoScreen_) {
        networkInfoScreen_->setNetworkInfo(state_.wifiConnected(), state_.ipAddress(), state_.netmask(), state_.gateway());
        networkInfoScreen_->setNeighbours(state_.neighbours());
    }
}

void AppUi::showMainScreen()
{
    clearRoot();

    colorPickerScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    mainScreen_ = std::make_unique<screens::MainScreen>(currentColor_);
    mainScreen_->setOnOpenColorPickerRequested([this]() { showColorPickerScreen(); });
    mainScreen_->setOnOpenNetworkInfoRequested([this]() { showNetworkInfoScreen(); });
    mainScreen_->setWifiConnected(state_.wifiConnected());
    mainScreen_->mount(root_);
}

void AppUi::showColorPickerScreen()
{
    clearRoot();

    mainScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    colorPickerScreen_ = std::make_unique<screens::ColorPickerScreen>(currentColor_);
    colorPickerScreen_->setOnColorChanged([this](const core::HsvColor& color) {
        currentColor_ = color;
    });
    colorPickerScreen_->setOnCloseRequested([this]() { showMainScreen(); });
    colorPickerScreen_->mount(root_);
}

void AppUi::showNetworkInfoScreen()
{
    clearRoot();

    mainScreen_.reset();
    colorPickerScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_ = std::make_unique<screens::NetworkInfoScreen>();
    networkInfoScreen_->setOnCloseRequested([this]() { showMainScreen(); });
    // Push current store state immediately on mount.
    networkInfoScreen_->setNetworkInfo(state_.wifiConnected(), state_.ipAddress(), state_.netmask(), state_.gateway());
    networkInfoScreen_->setNeighbours(state_.neighbours());
    networkInfoScreen_->mount(root_);
}

void AppUi::showWifiConfigScreen()
{
    clearRoot();

    mainScreen_.reset();
    colorPickerScreen_.reset();
    networkInfoScreen_.reset();
    wifiConfigScreen_ = std::make_unique<screens::WifiConfigScreen>();
    wifiConfigScreen_->mount(root_);
}

void AppUi::closeWifiConfigScreen()
{
    showMainScreen();
}

screens::WifiConfigScreen* AppUi::wifiConfigScreen()
{
    return wifiConfigScreen_.get();
}

void AppUi::setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway)
{
    const bool changed = state_.setNetworkInfo(connected, ip, netmask, gateway);
    if (!changed) {
        return;
    }
    // Visible-screen gating: only push to whichever screen is currently active.
    if (mainScreen_) {
        mainScreen_->setWifiConnected(state_.wifiConnected());
    }
    if (networkInfoScreen_) {
        networkInfoScreen_->setNetworkInfo(state_.wifiConnected(), state_.ipAddress(), state_.netmask(), state_.gateway());
    }
}

void AppUi::setNeighbours(const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours)
{
    const bool changed = state_.setNeighbours(neighbours);
    if (!changed) {
        return;
    }
    // Only update the neighbours list if NetworkInfoScreen is visible.
    if (networkInfoScreen_) {
        networkInfoScreen_->setNeighbours(state_.neighbours());
    }
}

} // namespace lightinator::ui

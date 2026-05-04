#include "ui/AppNavigator.h"

namespace lightinator::ui {

AppNavigator::AppNavigator(lv_obj_t* root, UiStateStore& state, core::HsvColor& currentColor,
                           const core::UiTheme& theme)
    : root_(root), factory_(state, currentColor, theme)
{
}

void AppNavigator::clearRoot()
{
    if (root_ == nullptr) {
        return;
    }
    // Notify unbind before destroying the screen.
    if (networkInfoScreen_ && onNetworkInfoScreenChanged_) {
        onNetworkInfoScreenChanged_(nullptr);
    }
    lv_obj_clean(root_);
}

void AppNavigator::pushStateToActiveScreen()
{
    // No-op: ScreenFactory seeds initial state; live updates come via AppUi setters.
}

void AppNavigator::showMainScreen()
{
    clearRoot();
    colorPickerScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    mainScreen_ = factory_.createMainScreen(
        [this]() { showColorPickerScreen(); },
        [this]() { showNetworkInfoScreen(); });
    mainScreen_->mount(root_);
}

void AppNavigator::showColorPickerScreen()
{
    clearRoot();
    mainScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    colorPickerScreen_ = factory_.createColorPickerScreen(
        [this]() { showMainScreen(); });
    colorPickerScreen_->mount(root_);
}

void AppNavigator::showNetworkInfoScreen()
{
    clearRoot();
    mainScreen_.reset();
    colorPickerScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_ = factory_.createNetworkInfoScreen(
        [this]() { showMainScreen(); });
    networkInfoScreen_->mount(root_);
    if (onNetworkInfoScreenChanged_) {
        onNetworkInfoScreenChanged_(networkInfoScreen_.get());
    }
}

void AppNavigator::showWifiConfigScreen()
{
    clearRoot();
    mainScreen_.reset();
    colorPickerScreen_.reset();
    networkInfoScreen_.reset();
    wifiConfigScreen_ = factory_.createWifiConfigScreen();
    wifiConfigScreen_->mount(root_);
}

void AppNavigator::closeWifiConfigScreen()
{
    showMainScreen();
}

screens::WifiConfigScreen* AppNavigator::wifiConfigScreen()
{
    return wifiConfigScreen_.get();
}

screens::MainScreen* AppNavigator::mainScreen()
{
    return mainScreen_.get();
}

screens::NetworkInfoScreen* AppNavigator::networkInfoScreen()
{
    return networkInfoScreen_.get();
}

void AppNavigator::setOnNetworkInfoScreenChanged(
    std::function<void(screens::NetworkInfoScreen*)> cb)
{
    onNetworkInfoScreenChanged_ = std::move(cb);
}

} // namespace lightinator::ui

#include "ui/AppNavigator.h"

namespace lightinator::ui {

AppNavigator::AppNavigator(lv_obj_t* root, UiStateStore& state, core::HsvColor& currentColor)
    : root_(root), state_(state), currentColor_(currentColor)
{
}

void AppNavigator::clearRoot()
{
    if (root_ == nullptr) {
        return;
    }
    lv_obj_clean(root_);
}

void AppNavigator::pushStateToActiveScreen()
{
    if (mainScreen_) {
        mainScreen_->setWifiConnected(state_.wifiConnected());
    }
    if (networkInfoScreen_) {
        networkInfoScreen_->setNetworkInfo(state_.wifiConnected(), state_.ipAddress(), state_.netmask(), state_.gateway());
        networkInfoScreen_->setNeighbours(state_.neighbours());
    }
}

void AppNavigator::showMainScreen()
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

void AppNavigator::showColorPickerScreen()
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

void AppNavigator::showNetworkInfoScreen()
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

void AppNavigator::showWifiConfigScreen()
{
    clearRoot();

    mainScreen_.reset();
    colorPickerScreen_.reset();
    networkInfoScreen_.reset();
    wifiConfigScreen_ = std::make_unique<screens::WifiConfigScreen>();
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

} // namespace lightinator::ui

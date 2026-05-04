#include "ui/AppNavigator.h"

namespace lightinator::ui {

AppNavigator::AppNavigator(lv_obj_t* root, UiStateStore& state, core::HsvColor& currentColor,
                           const core::UiTheme& theme,
                           std::function<bool(const core::UiTheme&)> onSaveTheme,
                           std::function<std::vector<core::UiTheme>()> onThemeList)
    : root_(root), factory_(state, currentColor, theme, std::move(onSaveTheme), std::move(onThemeList))
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
    themePreviewScreen_.reset();
    menuTestScreen_.reset();
    mainScreen_ = factory_.createMainScreen(
        [this]() { showColorPickerScreen(); },
        [this]() { showNetworkInfoScreen(); },
        [this]() { showThemePreviewScreen(); });
    mainScreen_->setOnOpenMenuTestRequested([this]() { showMenuTestScreen(); });
    mainScreen_->mount(root_);
}

void AppNavigator::showColorPickerScreen()
{
    clearRoot();
    mainScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    themePreviewScreen_.reset();
    menuTestScreen_.reset();
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
    themePreviewScreen_.reset();
    menuTestScreen_.reset();
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
    themePreviewScreen_.reset();
    menuTestScreen_.reset();
    wifiConfigScreen_ = factory_.createWifiConfigScreen();
    wifiConfigScreen_->mount(root_);
}

void AppNavigator::closeWifiConfigScreen()
{
    showMainScreen();
}

void AppNavigator::showThemePreviewScreen()
{
    clearRoot();
    mainScreen_.reset();
    colorPickerScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    menuTestScreen_.reset();
    themePreviewScreen_ = factory_.createThemePreviewScreen(
        [this]() { showMainScreen(); });
    themePreviewScreen_->mount(root_);
}


void AppNavigator::showMenuTestScreen()
{
    clearRoot();
    mainScreen_.reset();
    colorPickerScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    themePreviewScreen_.reset();
    menuTestScreen_.reset();
    menuTestScreen_ = factory_.createMenuTestScreen(
        [this]() { showMainScreen(); });
    menuTestScreen_->mount(root_);
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

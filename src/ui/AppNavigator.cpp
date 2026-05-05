#include "ui/AppNavigator.h"

namespace lightinator::ui {

AppNavigator::AppNavigator(lv_obj_t* root, UiStateStore& state, core::HsvColor& currentColor,
                           const core::UiTheme& theme,
                           std::function<bool(const core::UiTheme&)> onSaveTheme,
                           std::function<std::vector<core::UiTheme>()> onThemeList,
                           std::function<void(const core::UiTheme&)> onApplyTheme)
    : root_(root), factory_(state, currentColor, theme, std::move(onSaveTheme), std::move(onThemeList), std::move(onApplyTheme))
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
    activeScreen_ = ActiveScreen::Main;
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
    activeScreen_ = ActiveScreen::ColorPicker;
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
    activeScreen_ = ActiveScreen::NetworkInfo;
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
    activeScreen_ = ActiveScreen::WifiConfig;
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
    activeScreen_ = ActiveScreen::ThemePreview;
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
    activeScreen_ = ActiveScreen::MenuTest;
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

void AppNavigator::setTheme(const core::UiTheme& theme)
{
    factory_.setTheme(theme);

    // When theme changes are triggered from ThemePreviewScreen itself,
    // remounting here would destroy the current screen while its event
    // callback is still running. Apply the visual update in-place instead.
    if (activeScreen_ == ActiveScreen::ThemePreview) {
        if (themePreviewScreen_) {
            themePreviewScreen_->applyLiveTheme(theme);
        }
        return;
    }

    remountActiveScreen();
}

void AppNavigator::remountActiveScreen()
{
    switch (activeScreen_) {
    case ActiveScreen::Main:
        showMainScreen();
        break;
    case ActiveScreen::ColorPicker:
        showColorPickerScreen();
        break;
    case ActiveScreen::NetworkInfo:
        showNetworkInfoScreen();
        break;
    case ActiveScreen::WifiConfig:
        showWifiConfigScreen();
        break;
    case ActiveScreen::ThemePreview:
        showThemePreviewScreen();
        break;
    case ActiveScreen::MenuTest:
        showMenuTestScreen();
        break;
    case ActiveScreen::None:
    default:
        break;
    }
}

} // namespace lightinator::ui

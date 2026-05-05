#include "ui/AppNavigator.h"

namespace lightinator::ui {

AppNavigator::AppNavigator(lv_obj_t* root, UiStateStore& state, core::HsvColor& currentColor,
                           const core::UiTheme& theme,
                           std::function<bool(const core::UiTheme&)> onSaveTheme,
                           std::function<std::vector<core::UiTheme>()> onThemeList,
                           std::function<void(const core::UiTheme&)> onApplyTheme,
                           std::function<void(int&, int&)> onLoadSettings,
                           std::function<bool(int, int)> onSaveSettings,
                           std::function<void(int)> onPreviewBrightness,
                           std::function<bool(const core::TouchCalibrationCapture&)> onSaveTouchCalibration)
        : root_(root),
          factory_(state, currentColor, theme,
                   std::move(onSaveTheme), std::move(onThemeList), std::move(onApplyTheme),
                   std::move(onLoadSettings), std::move(onSaveSettings), std::move(onPreviewBrightness),
                   std::move(onSaveTouchCalibration))
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

void AppNavigator::resetScreenInstances()
{
    mainScreen_.reset();
    colorPickerScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    themePreviewScreen_.reset();
    touchCalibrationScreen_.reset();
    settingsScreen_.reset();
    themeSelectorScreen_.reset();
    menuTestScreen_.reset();
}

void AppNavigator::showMainScreen()
{
    activeScreen_ = ActiveScreen::Main;
    clearRoot();
    resetScreenInstances();
    mainScreen_ = factory_.createMainScreen(
        [this]() { showColorPickerScreen(); },
        [this]() { showNetworkInfoScreen(); },
        [this]() { showThemeSelectorScreen(); });
    mainScreen_->setOnOpenSettingsRequested([this]() { showSettingsScreen(); });
    mainScreen_->mount(root_);
}

void AppNavigator::showColorPickerScreen()
{
    activeScreen_ = ActiveScreen::ColorPicker;
    clearRoot();
    resetScreenInstances();
    colorPickerScreen_ = factory_.createColorPickerScreen(
        [this]() { showMainScreen(); });
    colorPickerScreen_->mount(root_);
}

void AppNavigator::showNetworkInfoScreen()
{
    activeScreen_ = ActiveScreen::NetworkInfo;
    clearRoot();
    resetScreenInstances();
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
    resetScreenInstances();
    wifiConfigScreen_ = factory_.createWifiConfigScreen();
    wifiConfigScreen_->mount(root_);
}

void AppNavigator::closeWifiConfigScreen()
{
    showMainScreen();
}

void AppNavigator::showThemePreviewScreen()
{
    showThemeSelectorScreen();
}

void AppNavigator::showThemeSelectorScreen()
{
    activeScreen_ = ActiveScreen::ThemeSelector;
    clearRoot();
    resetScreenInstances();
    themeSelectorScreen_ = factory_.createThemeSelectorScreen(
        [this]() { showMainScreen(); },
        [this](const core::UiTheme& t, const String& name) {
            showThemeEditorScreen(t, name);
        });
    themeSelectorScreen_->mount(root_);
}

void AppNavigator::showThemeEditorScreen(const core::UiTheme& themeToEdit,
                                          const String& suggestedName)
{
    activeScreen_ = ActiveScreen::ThemeEditor;
    clearRoot();
    resetScreenInstances();
    themePreviewScreen_ = factory_.createThemeEditorScreen(
        themeToEdit, suggestedName,
        [this]() { showThemeSelectorScreen(); });
    themePreviewScreen_->mount(root_);
}


void AppNavigator::showMenuTestScreen()
{
    activeScreen_ = ActiveScreen::MenuTest;
    clearRoot();
    resetScreenInstances();
    menuTestScreen_ = factory_.createMenuTestScreen(
        [this]() { showMainScreen(); });
    menuTestScreen_->mount(root_);
}

void AppNavigator::showSettingsScreen()
{
    activeScreen_ = ActiveScreen::Settings;
    clearRoot();
    resetScreenInstances();
    settingsScreen_ = factory_.createSettingsScreen(
        [this]() { showMainScreen(); },
        [this]() { showTouchCalibrationScreen(); });
    settingsScreen_->mount(root_);
}

void AppNavigator::showTouchCalibrationScreen()
{
    activeScreen_ = ActiveScreen::TouchCalibration;
    clearRoot();
    resetScreenInstances();
    touchCalibrationScreen_ = factory_.createTouchCalibrationScreen(
        [this]() { showSettingsScreen(); });
    touchCalibrationScreen_->mount(root_);
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

    if (activeScreen_ == ActiveScreen::ThemeEditor) {
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
        showThemeSelectorScreen();
        break;
    case ActiveScreen::ThemeSelector:
        showThemeSelectorScreen();
        break;
    case ActiveScreen::ThemeEditor:
        // Editor can't be remounted generically (needs theme + suggestedName);
        // fall back to selector.
        showThemeSelectorScreen();
        break;
    case ActiveScreen::Settings:
        showSettingsScreen();
        break;
    case ActiveScreen::TouchCalibration:
        showTouchCalibrationScreen();
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

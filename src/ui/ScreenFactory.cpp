#include "ui/ScreenFactory.h"
#include "ui/screens/ThemePreviewScreen.h"
#include "ui/screens/ThemeSelectorScreen.h"

namespace lightinator::ui {

ScreenFactory::ScreenFactory(UiStateStore& state, core::HsvColor& currentColor,
                             const core::UiTheme& theme,
                             std::function<bool(const core::UiTheme&)> onSaveTheme,
                             std::function<std::vector<core::UiTheme>()> onThemeList,
                             std::function<void(const core::UiTheme&)> onApplyTheme,
                             std::function<void(int&, int&)> onLoadSettings,
                             std::function<bool(int, int)> onSaveSettings,
                                                         std::function<void(int)> onPreviewBrightness,
                             std::function<bool(const core::TouchCalibrationCapture&)> onSaveTouchCalibration)
        : state_(state),
          currentColor_(currentColor),
          theme_(theme),
          onSaveTheme_(std::move(onSaveTheme)),
          onThemeList_(std::move(onThemeList)),
          onApplyTheme_(std::move(onApplyTheme)),
          onLoadSettings_(std::move(onLoadSettings)),
          onSaveSettings_(std::move(onSaveSettings)),
          onPreviewBrightness_(std::move(onPreviewBrightness)),
          onSaveTouchCalibration_(std::move(onSaveTouchCalibration))
{
}

void ScreenFactory::setTheme(const core::UiTheme& theme)
{
    theme_ = theme;
}

std::unique_ptr<screens::MainScreen> ScreenFactory::createMainScreen(
    std::function<void()> onOpenColorPicker,
    std::function<void()> onOpenNetworkInfo,
    std::function<void()> onOpenThemePreview)
{
    auto screen = std::make_unique<screens::MainScreen>(currentColor_, theme_);
    screen->setOnOpenColorPickerRequested(std::move(onOpenColorPicker));
    screen->setOnOpenNetworkInfoRequested(std::move(onOpenNetworkInfo));
    screen->setOnOpenThemePreviewRequested(std::move(onOpenThemePreview));
    screen->setWifiConnected(state_.wifiConnected());
    return screen;
}

std::unique_ptr<screens::ColorPickerScreen> ScreenFactory::createColorPickerScreen(
    std::function<void()> onClose)
{
    auto screen = std::make_unique<screens::ColorPickerScreen>(currentColor_, theme_);
    screen->setOnColorChanged([this](const core::HsvColor& color) {
        currentColor_ = color;
    });
    screen->setOnCloseRequested(std::move(onClose));
    return screen;
}

std::unique_ptr<screens::NetworkInfoScreen> ScreenFactory::createNetworkInfoScreen(
    std::function<void()> onClose)
{
    auto screen = std::make_unique<screens::NetworkInfoScreen>(theme_);
    screen->setOnCloseRequested(std::move(onClose));
    // Initial state is seeded by NetworkInfoPresenter::bind() after mount.
    return screen;
}

std::unique_ptr<screens::WifiConfigScreen> ScreenFactory::createWifiConfigScreen()
{
    return std::make_unique<screens::WifiConfigScreen>(theme_);
}


std::unique_ptr<screens::MenuTestScreen> ScreenFactory::createMenuTestScreen(
    std::function<void()> onClose)
{
    auto screen = std::make_unique<screens::MenuTestScreen>(theme_);
    screen->setOnCloseRequested(std::move(onClose));
    return screen;
}

std::unique_ptr<screens::SettingsScreen> ScreenFactory::createSettingsScreen(
    std::function<void()> onClose,
    std::function<void()> onOpenTouchCalibration)
{
    auto screen = std::make_unique<screens::SettingsScreen>(theme_);
    screen->setOnCloseRequested(std::move(onClose));

    int brightness = 80;
    int timeout = 30;
    if (onLoadSettings_) {
        onLoadSettings_(brightness, timeout);
    }
    screen->setInitialValues(brightness, timeout);
    screen->setOnSaveRequested(onSaveSettings_);
    screen->setOnBrightnessPreviewRequested(onPreviewBrightness_);
    screen->setOnOpenTouchCalibrationRequested(std::move(onOpenTouchCalibration));
    return screen;
}

std::unique_ptr<screens::TouchCalibrationScreen> ScreenFactory::createTouchCalibrationScreen(
    std::function<void()> onClose,
    std::function<void()> onSaved)
{
    auto screen = std::make_unique<screens::TouchCalibrationScreen>(theme_);
    screen->setOnCloseRequested(std::move(onClose));
    screen->setOnSaveRequested([this, onSaved](const core::TouchCalibrationCapture& capture) {
        bool ok = true;
        if (onSaveTouchCalibration_) {
            ok = onSaveTouchCalibration_(capture);
        }
        if (ok && onSaved) {
            onSaved();
        }
        return ok;
    });
    return screen;
}
std::unique_ptr<screens::ThemePreviewScreen> ScreenFactory::createThemePreviewScreen(
    std::function<void()> onClose)
{
    // Legacy entry point: open editor with the current theme as a user copy.
    const String suggestedName = theme_.isBuiltin
        ? (theme_.name + "-user")
        : (theme_.name + "-edit");
    auto screen = std::make_unique<screens::ThemePreviewScreen>(theme_, suggestedName);
    screen->setOnCloseRequested(std::move(onClose));
    screen->setOnSaveRequested(onSaveTheme_);
    screen->setOnThemeApplyRequested(onApplyTheme_);
    return screen;
}

std::unique_ptr<screens::ThemeSelectorScreen> ScreenFactory::createThemeSelectorScreen(
    std::function<void()> onClose,
    std::function<void(const core::UiTheme&, const String&)> onEdit)
{
    auto screen = std::make_unique<screens::ThemeSelectorScreen>(theme_);
    screen->setOnCloseRequested(std::move(onClose));
    screen->setOnThemeListRequested(onThemeList_);
    screen->setOnThemeApplyRequested(onApplyTheme_);
    screen->setOnThemeEditRequested(std::move(onEdit));
    return screen;
}

std::unique_ptr<screens::ThemePreviewScreen> ScreenFactory::createThemeEditorScreen(
    const core::UiTheme& themeToEdit,
    const String& suggestedName,
    std::function<void()> onClose)
{
    auto screen = std::make_unique<screens::ThemePreviewScreen>(themeToEdit, suggestedName);
    screen->setOnCloseRequested(std::move(onClose));
    screen->setOnSaveRequested(onSaveTheme_);
    screen->setOnThemeApplyRequested(onApplyTheme_);
    return screen;
}

} // namespace lightinator::ui

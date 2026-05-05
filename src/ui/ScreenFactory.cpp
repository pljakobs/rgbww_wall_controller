#include "ui/ScreenFactory.h"
#include "ui/screens/ThemePreviewScreen.h"
#include "ui/screens/ThemeSelectorScreen.h"

namespace lightinator::ui {

ScreenFactory::ScreenFactory(UiStateStore& state, core::HsvColor& currentColor,
                             const core::UiTheme& theme,
                                                         std::function<bool(const core::UiTheme&)> onSaveTheme,
                                                         std::function<std::vector<core::UiTheme>()> onThemeList,
                                                         std::function<void(const core::UiTheme&)> onApplyTheme)
        : state_(state),
            currentColor_(currentColor),
            theme_(theme),
            onSaveTheme_(std::move(onSaveTheme)),
            onThemeList_(std::move(onThemeList)),
            onApplyTheme_(std::move(onApplyTheme))
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

#include "ui/ScreenFactory.h"

namespace lightinator::ui {

ScreenFactory::ScreenFactory(UiStateStore& state, core::HsvColor& currentColor,
                             const core::UiTheme& theme)
    : state_(state), currentColor_(currentColor), theme_(theme)
{
}

std::unique_ptr<screens::MainScreen> ScreenFactory::createMainScreen(
    std::function<void()> onOpenColorPicker,
    std::function<void()> onOpenNetworkInfo)
{
    auto screen = std::make_unique<screens::MainScreen>(currentColor_, theme_);
    screen->setOnOpenColorPickerRequested(std::move(onOpenColorPicker));
    screen->setOnOpenNetworkInfoRequested(std::move(onOpenNetworkInfo));
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

} // namespace lightinator::ui

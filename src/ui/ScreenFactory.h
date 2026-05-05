#pragma once

#include <lvgl.h>

#include <functional>
#include <memory>
#include <vector>

#include "ui/core/HsvColor.h"
#include "ui/core/UiTheme.h"
#include "ui/UiStateStore.h"
#include "ui/screens/ColorPickerScreen.h"
#include "ui/screens/MainScreen.h"
#include "ui/screens/NetworkInfoScreen.h"
#include "ui/screens/ThemePreviewScreen.h"
#include "ui/screens/WifiConfigScreen.h"
#include "ui/screens/MenuTestScreen.h"

namespace lightinator::ui {

/**
 * ScreenFactory — screen construction and callback injection only.
 *
 * Each create* method allocates a screen, wires all callbacks, and
 * returns the fully-ready unique_ptr. Mounting is left to the caller
 * (AppNavigator) so factory and mount-point remain decoupled.
 */
class ScreenFactory {
public:
    ScreenFactory(UiStateStore& state, core::HsvColor& currentColor, const core::UiTheme& theme,
                  std::function<bool(const core::UiTheme&)> onSaveTheme,
                  std::function<std::vector<core::UiTheme>()> onThemeList,
                  std::function<void(const core::UiTheme&)> onApplyTheme);

    void setTheme(const core::UiTheme& theme);

    std::unique_ptr<screens::MainScreen> createMainScreen(
        std::function<void()> onOpenColorPicker,
        std::function<void()> onOpenNetworkInfo,
        std::function<void()> onOpenThemePreview);

    std::unique_ptr<screens::ColorPickerScreen> createColorPickerScreen(
        std::function<void()> onClose);

    std::unique_ptr<screens::NetworkInfoScreen> createNetworkInfoScreen(
        std::function<void()> onClose);

    std::unique_ptr<screens::WifiConfigScreen> createWifiConfigScreen();

    std::unique_ptr<screens::ThemePreviewScreen> createThemePreviewScreen(
        std::function<void()> onClose);

    std::unique_ptr<screens::MenuTestScreen> createMenuTestScreen(
        std::function<void()> onClose);

private:
    UiStateStore&      state_;
    core::HsvColor&    currentColor_;
    core::UiTheme      theme_;
    std::function<bool(const core::UiTheme&)> onSaveTheme_;
    std::function<std::vector<core::UiTheme>()> onThemeList_;
    std::function<void(const core::UiTheme&)> onApplyTheme_;
};

} // namespace lightinator::ui

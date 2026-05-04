#pragma once

#include <lvgl.h>

#include <functional>
#include <memory>

#include "ui/core/HsvColor.h"
#include "ui/core/UiTheme.h"
#include "ui/UiStateStore.h"
#include "ui/screens/ColorPickerScreen.h"
#include "ui/screens/MainScreen.h"
#include "ui/screens/NetworkInfoScreen.h"
#include "ui/screens/WifiConfigScreen.h"

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
    ScreenFactory(UiStateStore& state, core::HsvColor& currentColor, const core::UiTheme& theme);

    std::unique_ptr<screens::MainScreen> createMainScreen(
        std::function<void()> onOpenColorPicker,
        std::function<void()> onOpenNetworkInfo);

    std::unique_ptr<screens::ColorPickerScreen> createColorPickerScreen(
        std::function<void()> onClose);

    std::unique_ptr<screens::NetworkInfoScreen> createNetworkInfoScreen(
        std::function<void()> onClose);

    std::unique_ptr<screens::WifiConfigScreen> createWifiConfigScreen();

private:
    UiStateStore&      state_;
    core::HsvColor&    currentColor_;
    core::UiTheme      theme_;
};

} // namespace lightinator::ui

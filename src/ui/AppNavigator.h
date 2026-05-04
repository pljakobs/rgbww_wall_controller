#pragma once

#include <lvgl.h>

#include <functional>
#include <memory>

#include "ui/core/HsvColor.h"
#include "ui/UiStateStore.h"
#include "ui/ScreenFactory.h"
#include "ui/screens/MainScreen.h"
#include "ui/screens/ColorPickerScreen.h"
#include "ui/screens/NetworkInfoScreen.h"
#include "ui/screens/WifiConfigScreen.h"

namespace lightinator::ui {

/**
 * AppNavigator — screen transition logic only.
 *
 * Owns all screen instance pointers. Drives showX / clearRoot.
 * Delegates screen construction to ScreenFactory.
 * Exposes raw accessors for visible-screen gating.
 */
class AppNavigator {
public:
    AppNavigator(lv_obj_t* root, UiStateStore& state, core::HsvColor& currentColor);

    void showMainScreen();
    void showColorPickerScreen();
    void showNetworkInfoScreen();
    void showWifiConfigScreen();
    void closeWifiConfigScreen();

    screens::WifiConfigScreen*   wifiConfigScreen();
    screens::MainScreen*         mainScreen();
    screens::NetworkInfoScreen*  networkInfoScreen();

    /// Push current store state to whichever screen is active (called after navigation).
    void pushStateToActiveScreen();

    /// Register a callback that fires with the active NetworkInfoScreen pointer when
    /// the screen is shown, or nullptr when it is hidden (navigated away).
    void setOnNetworkInfoScreenChanged(
        std::function<void(screens::NetworkInfoScreen*)> cb);

private:
    void clearRoot();

    lv_obj_t*        root_;
    ScreenFactory    factory_;

    std::unique_ptr<screens::MainScreen>        mainScreen_;
    std::unique_ptr<screens::ColorPickerScreen> colorPickerScreen_;
    std::unique_ptr<screens::WifiConfigScreen>  wifiConfigScreen_;
    std::unique_ptr<screens::NetworkInfoScreen> networkInfoScreen_;

    std::function<void(screens::NetworkInfoScreen*)> onNetworkInfoScreenChanged_;
};

} // namespace lightinator::ui

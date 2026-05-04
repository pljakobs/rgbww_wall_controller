#pragma once

#include <lvgl.h>

#include <memory>
#include <vector>

#include "ui/core/HsvColor.h"
#include "ui/UiStateStore.h"
#include "ui/screens/ColorPickerScreen.h"
#include "ui/screens/MainScreen.h"
#include "ui/screens/NetworkInfoScreen.h"
#include "ui/screens/WifiConfigScreen.h"

namespace lightinator::ui {

/**
 * AppNavigator — screen transition logic only.
 *
 * Owns all screen instance pointers. Drives showX / clearRoot.
 * Exposes raw accessors so callers can perform visible-screen gating
 * without AppNavigator needing to know about state semantics.
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

private:
    void clearRoot();

    lv_obj_t*        root_;
    UiStateStore&    state_;
    core::HsvColor&  currentColor_;

    std::unique_ptr<screens::MainScreen>        mainScreen_;
    std::unique_ptr<screens::ColorPickerScreen> colorPickerScreen_;
    std::unique_ptr<screens::WifiConfigScreen>  wifiConfigScreen_;
    std::unique_ptr<screens::NetworkInfoScreen> networkInfoScreen_;
};

} // namespace lightinator::ui

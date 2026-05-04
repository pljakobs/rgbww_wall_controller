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

class AppUi {
public:
    ~AppUi();

    bool init();
    void tickAnimation();

    void showWifiConfigScreen();
    void closeWifiConfigScreen();
    screens::WifiConfigScreen* wifiConfigScreen();

    // State setters: delegate to UiStateStore and push to active screen only on change.
    void setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway);
    void setNeighbours(const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours);

private:
    void clearRoot();
    void showMainScreen();
    void showColorPickerScreen();
    void showNetworkInfoScreen();

    // Push current store state to whatever screen is active (called after navigation).
    void pushStateToActiveScreen();

    core::HsvColor currentColor_;
    UiStateStore state_;

    lv_obj_t* screen_ = nullptr;
    lv_obj_t* root_ = nullptr;
    bool initialized_ = false;

    std::unique_ptr<screens::MainScreen> mainScreen_;
    std::unique_ptr<screens::ColorPickerScreen> colorPickerScreen_;
    std::unique_ptr<screens::WifiConfigScreen> wifiConfigScreen_;
    std::unique_ptr<screens::NetworkInfoScreen> networkInfoScreen_;
};

} // namespace lightinator::ui

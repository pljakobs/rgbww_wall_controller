#pragma once

#include <lvgl.h>

#include <memory>
#include <vector>

#include "ui/core/HsvColor.h"
#include "ui/UiStateStore.h"
#include "ui/AppNavigator.h"
#include "ui/NetworkInfoPresenter.h"
#include "ui/screens/NetworkInfoScreen.h"
#include "ui/screens/WifiConfigScreen.h"

namespace lightinator::ui {

/**
 * AppUi — LVGL root ownership and public UI API.
 *
 * Owns the LVGL screen/root objects and the UiStateStore.
 * Delegates all screen navigation to AppNavigator.
 */
class AppUi {
public:
    AppUi();
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
    core::HsvColor currentColor_;
    UiStateStore state_;
    NetworkInfoPresenter networkInfoPresenter_;

    lv_obj_t* screen_ = nullptr;
    lv_obj_t* root_ = nullptr;
    bool initialized_ = false;

    std::unique_ptr<AppNavigator> navigator_;
};

} // namespace lightinator::ui

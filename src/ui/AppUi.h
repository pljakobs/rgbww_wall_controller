#pragma once

#include <lvgl.h>

#include <functional>
#include <memory>
#include <vector>

#include "ui/core/HsvColor.h"
#include "ui/core/TouchCalibrationData.h"
#include "ui/core/UiTheme.h"
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
    void setTheme(const core::UiTheme& theme);
    void setOnThemeSaveRequested(std::function<bool(const core::UiTheme&)> callback);
    void setOnThemeListRequested(std::function<std::vector<core::UiTheme>()> callback);
    void setOnThemeApplyRequested(std::function<void(const core::UiTheme&)> callback);
    void setOnSettingsLoadRequested(std::function<void(int&, int&)> callback);
    void setOnSettingsSaveRequested(std::function<bool(int, int)> callback);
    void setOnBrightnessPreviewRequested(std::function<void(int)> callback);
    void setOnTouchCalibrationSaveRequested(std::function<bool(const core::TouchCalibrationCapture&)> callback);

    void showWifiConfigScreen();
    void closeWifiConfigScreen();
    screens::WifiConfigScreen* wifiConfigScreen();

    // State setters: delegate to UiStateStore and push to active screen only on change.
    void setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway);
    void setNeighbours(const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours);

private:
    core::HsvColor currentColor_;
    core::UiTheme  theme_ = core::nordicDarkTheme();
    UiStateStore state_;
    NetworkInfoPresenter networkInfoPresenter_;

    lv_obj_t* screen_ = nullptr;
    lv_obj_t* root_ = nullptr;
    bool initialized_ = false;
    std::function<bool(const core::UiTheme&)> onThemeSaveRequested_;
    std::function<std::vector<core::UiTheme>()> onThemeListRequested_;
    std::function<void(const core::UiTheme&)> onThemeApplyRequested_;
    std::function<void(int&, int&)> onSettingsLoadRequested_;
    std::function<bool(int, int)> onSettingsSaveRequested_;
    std::function<void(int)> onBrightnessPreviewRequested_;
    std::function<bool(const core::TouchCalibrationCapture&)> onTouchCalibrationSaveRequested_;

    std::unique_ptr<AppNavigator> navigator_;
};

} // namespace lightinator::ui

#include "ui/AppUi.h"

#include <memory>

#include "AppPolicy.h"

namespace lightinator::ui {

AppUi::~AppUi() = default;

AppUi::AppUi()
    : networkInfoPresenter_(state_)
{
}

bool AppUi::init()
{
    if (initialized_) {
        return true;
    }

    screen_ = lv_scr_act();
    if (screen_ == nullptr) {
        return false;
    }

    lv_obj_set_style_bg_color(screen_, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, 0);

    root_ = lv_obj_create(screen_);
    lv_obj_remove_style_all(root_);
    lv_obj_set_size(root_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(root_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(root_, 0, 0);

    currentColor_ = core::clampHsv(currentColor_);
    navigator_ = std::make_unique<AppNavigator>(
        root_, state_, currentColor_, theme_, onThemeSaveRequested_, onThemeListRequested_,
        [this](const core::UiTheme& theme) {
            setTheme(theme);
            // Persist only concrete theme selections (selector apply).
            // Editor live preview may emit themes with empty ids.
            if (onThemeApplyRequested_ && theme.id.length() != 0) {
                onThemeApplyRequested_(theme);
            }
        },
        onSettingsLoadRequested_ ? onSettingsLoadRequested_ : [](int& brightness, int& timeout) {
            brightness = lightinator::policy::kDefaultBrightnessPercent;
            timeout = lightinator::policy::kDefaultBacklightTimeoutSeconds;
        },
        onSettingsSaveRequested_ ? onSettingsSaveRequested_ : [](int, int) {
            return true;
        },
        onBrightnessPreviewRequested_,
        [this](const core::TouchCalibrationCapture& capture) {
            if (!onTouchCalibrationSaveRequested_) {
                return true;
            }
            return onTouchCalibrationSaveRequested_(capture);
        });
    navigator_->setOnNetworkInfoScreenChanged(
        [this](screens::NetworkInfoScreen* screen) {
            networkInfoPresenter_.bind(screen);
        });
    navigator_->showMainScreen();
    initialized_ = true;
    return true;
}

void AppUi::tickAnimation()
{
    // The app is event-driven; timer ticks are currently unused.
}

void AppUi::setTheme(const core::UiTheme& theme)
{
    theme_ = theme;
    if (screen_ != nullptr) {
        lv_obj_set_style_bg_color(screen_, theme_.colors.contentBg, 0);
    }
    if (initialized_ && navigator_) {
        navigator_->setTheme(theme_);
    }
}

void AppUi::setOnThemeSaveRequested(std::function<bool(const core::UiTheme&)> callback)
{
    onThemeSaveRequested_ = std::move(callback);
}

void AppUi::setOnThemeListRequested(std::function<std::vector<core::UiTheme>()> callback)
{
    onThemeListRequested_ = std::move(callback);
}

void AppUi::setOnThemeApplyRequested(std::function<void(const core::UiTheme&)> callback)
{
    onThemeApplyRequested_ = std::move(callback);
}

void AppUi::setOnSettingsLoadRequested(std::function<void(int&, int&)> callback)
{
    onSettingsLoadRequested_ = std::move(callback);
}

void AppUi::setOnSettingsSaveRequested(std::function<bool(int, int)> callback)
{
    onSettingsSaveRequested_ = std::move(callback);
}

void AppUi::setOnBrightnessPreviewRequested(std::function<void(int)> callback)
{
    onBrightnessPreviewRequested_ = std::move(callback);
}

void AppUi::setOnTouchCalibrationSaveRequested(std::function<bool(const core::TouchCalibrationCapture&)> callback)
{
    onTouchCalibrationSaveRequested_ = std::move(callback);
}

void AppUi::showWifiConfigScreen()
{
    navigator_->showWifiConfigScreen();
}

void AppUi::closeWifiConfigScreen()
{
    navigator_->closeWifiConfigScreen();
}

screens::WifiConfigScreen* AppUi::wifiConfigScreen()
{
    return navigator_->wifiConfigScreen();
}

void AppUi::setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway)
{
    const bool changed = state_.setNetworkInfo(connected, ip, netmask, gateway);
    if (!changed) {
        return;
    }
    // Update the wifi icon on main screen if visible.
    if (navigator_->mainScreen()) {
        navigator_->mainScreen()->setWifiConnected(state_.wifiConnected());
    }
    // Delegate network info rendering to the presenter.
    networkInfoPresenter_.onNetworkInfoChanged();
}

void AppUi::setNeighbours(const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours)
{
    const bool changed = state_.setNeighbours(neighbours);
    if (!changed) {
        return;
    }
    // Delegate neighbour list rendering to the presenter.
    networkInfoPresenter_.onNeighboursChanged();
}

} // namespace lightinator::ui

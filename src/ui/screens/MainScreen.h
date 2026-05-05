#pragma once

#include <functional>
#include <memory>

#include <lvgl.h>
#include <lvglCpp.h>

#include "ui/core/HsvColor.h"
#include "ui/core/LvObjRef.h"
#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/AntaresImage.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

class MainScreen : public core::Screen {
public:
    explicit MainScreen(core::HsvColor color, const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;

    void setOnOpenColorPickerRequested(std::function<void()> callback);
    void setOnOpenNetworkInfoRequested(std::function<void()> callback);
    void setOnOpenThemePreviewRequested(std::function<void()> callback);
    void setOnOpenSettingsRequested(std::function<void()> callback);
    void setOnOpenMenuTestRequested(std::function<void()> callback);
    void setWifiConnected(bool connected);

private:
    static void onMenuThemeEvent(lv_event_t* event);
    static void onMenuColorPickerEvent(lv_event_t* event);
    static void onMenuNetworkInfoEvent(lv_event_t* event);
    static void onMenuSettingsEvent(lv_event_t* event);
    static void onMenuMenuTestEvent(lv_event_t* event);
    static void onMenuDismissEvent(lv_event_t* event);

    void showBurgerMenu();
    void hideBurgerMenu();
    void updateBurgerMenuGeometry();

    core::HsvColor color_;
    core::UiTheme theme_;
    std::function<void()> onOpenColorPickerRequested_;
    std::function<void()> onOpenNetworkInfoRequested_;
    std::function<void()> onOpenThemePreviewRequested_;
    std::function<void()> onOpenSettingsRequested_;
    std::function<void()> onOpenMenuTestRequested_;
    bool wifiConnected_ = false;

    core::LvObjRef burgerMenuOverlay_;
    core::LvObjRef burgerMenuPanel_;
    core::LvObjRef backgroundImage_;

    std::unique_ptr<DecoratedScreen> decorated_;
};

} // namespace lightinator::ui::screens

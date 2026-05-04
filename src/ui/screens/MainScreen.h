#pragma once

#include <functional>
#include <memory>

#include <lvgl.h>
#include <lvglCpp.h>

#include "ui/core/HsvColor.h"
#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

class MainScreen : public core::Screen {
public:
    explicit MainScreen(core::HsvColor color, const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;
    void setColorPreview(core::HsvColor color);

    void setOnOpenColorPickerRequested(std::function<void()> callback);
    void setOnOpenNetworkInfoRequested(std::function<void()> callback);
    void setOnOpenThemePreviewRequested(std::function<void()> callback);
    void setWifiConnected(bool connected);

private:
    static void onOpenButtonEvent(lv_event_t* event);
    static void onMenuItemEvent(lv_event_t* event);
    static void onMenuDismissEvent(lv_event_t* event);

    void showBurgerMenu();
    void hideBurgerMenu();

    core::HsvColor color_;
    core::UiTheme theme_;
    std::function<void()> onOpenColorPickerRequested_;
    std::function<void()> onOpenNetworkInfoRequested_;
    std::function<void()> onOpenThemePreviewRequested_;
    bool wifiConnected_ = false;

    lv_obj_t* burgerMenuPanel_ = nullptr;

    std::unique_ptr<DecoratedScreen> decorated_;
    std::unique_ptr<lvgl::widget::Object> bodyLayout_;
    std::unique_ptr<lvgl::widget::Object> previewSwatch_;
    std::unique_ptr<lvgl::widget::Label> previewLabel_;
    std::unique_ptr<lvgl::widget::Button> openButton_;
};

} // namespace lightinator::ui::screens

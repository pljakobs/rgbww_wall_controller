#pragma once

#include <functional>
#include <memory>

#include <lvgl.h>
#include <lvglCpp.h>

#include "ui/core/Screen.h"
#include "ui/core/SlotHost.h"

namespace lightinator::ui::screens {

struct HeaderOptions {
    const char* text = "Screen";
    lv_coord_t height = 60;
    lv_coord_t titleWidth = 100;
    lv_color_t color = lv_color_hex(0x4189C8);
    const lv_font_t* font = &lv_font_montserrat_36;
    bool showClose = false;
    bool showBurger = false;
};

class DecoratedScreen : public core::Screen, public core::SlotHost {
public:
    explicit DecoratedScreen(HeaderOptions options);

    void mount(lv_obj_t* parent) override;
    void setBody(std::unique_ptr<core::Widget> widget) override;

    lv_obj_t* bodySlot() const;
    void setHeaderStatusIcon(const char* iconText, bool visible);
    void setOnHeaderStatusIconTapped(std::function<void()> callback);

    void setOnCloseRequested(std::function<void()> callback);
    void setOnBurgerTapped(std::function<void()> callback);

private:
    static void onCloseButtonEvent(lv_event_t* event);
    static void onBurgerButtonEvent(lv_event_t* event);
    static void onHeaderStatusIconEvent(lv_event_t* event);

    HeaderOptions options_;
    std::function<void()> onCloseRequested_;
    std::function<void()> onBurgerTapped_;
    std::function<void()> onHeaderStatusIconTapped_;

    std::unique_ptr<lvgl::widget::Object> rootObj_;
    std::unique_ptr<lvgl::widget::Object> headerObj_;
    std::unique_ptr<lvgl::widget::Object> bodyObj_;
    std::unique_ptr<lvgl::widget::Button> burgerButton_;
    std::unique_ptr<lvgl::widget::Label> titleLabel_;
    std::unique_ptr<lvgl::widget::Object> statusIconBox_;
    std::unique_ptr<lvgl::widget::Label> statusIconLabel_;
    std::unique_ptr<lvgl::widget::Button> closeButton_;
    std::unique_ptr<core::Widget> bodyWidget_;
};

} // namespace lightinator::ui::screens

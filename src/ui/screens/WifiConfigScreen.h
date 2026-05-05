#pragma once

#include <SmingCore.h>

#include <functional>
#include <memory>

#include <lvgl.h>
#include <lvglCpp.h>

#include "ui/core/Screen.h"
#include "ui/core/LvObjRef.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

class WifiConfigScreen : public core::Screen {
public:
    explicit WifiConfigScreen(const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;

    void setNetworks(const BssList& networks);
    void setStatusText(const String& text);

    void setOnConnectRequested(std::function<void(const String&, const String&)> callback);

private:
    static void onNetworkButtonEvent(lv_event_t* event);
    static void onConnectButtonEvent(lv_event_t* event);
    static void onTextAreaEvent(lv_event_t* event);
    static void onKeyboardEvent(lv_event_t* event);
    static void onPasswordToggleEvent(lv_event_t* event);

    String selectedSsidFromList() const;
    void showKeyboardFor(lv_obj_t* textArea);
    void hideKeyboard();

    std::function<void(const String&, const String&)> onConnectRequested_;

    core::UiTheme theme_;
    std::unique_ptr<DecoratedScreen> decorated_;
    std::unique_ptr<lvgl::widget::Object> bodyLayout_;
    std::unique_ptr<lvgl::widget::Label> networksLabel_;
    std::unique_ptr<lvgl::widget::List> networksList_;
    std::unique_ptr<lvgl::widget::Label> ssidLabel_;
    std::unique_ptr<lvgl::widget::TextArea> ssidField_;
    std::unique_ptr<lvgl::widget::Label> passwordLabel_;
    std::unique_ptr<lvgl::widget::Object> passwordRow_;
    std::unique_ptr<lvgl::widget::TextArea> passwordField_;
    std::unique_ptr<lvgl::widget::Button> passwordToggleBtn_;
    bool passwordVisible_{false};
    std::unique_ptr<lvgl::widget::Object> actionsRow_;
    std::unique_ptr<lvgl::widget::Button> connectButton_;
    std::unique_ptr<lvgl::widget::TextArea> statusField_;
    core::LvObjRef keyboard_;
};

} // namespace lightinator::ui::screens

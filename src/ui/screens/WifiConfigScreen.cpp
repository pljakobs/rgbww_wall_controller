#include "ui/screens/WifiConfigScreen.h"

#include <algorithm>

namespace lightinator::ui::screens {

WifiConfigScreen::WifiConfigScreen(const core::UiTheme& theme) : theme_(theme)
{
}

void WifiConfigScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text = "wifi config";
    header.showClose = false;
    header.height = theme_.headerHeight;
    header.color  = theme_.colors.headerBg;
    header.font   = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
    decorated_->mount(parent);
    setRoot(decorated_->root());

    bodyLayout_ = std::make_unique<lvgl::widget::Object>(decorated_->bodySlot());
    bodyLayout_->SetSize(lv_pct(100), lv_pct(100))
        ->SetStyleBgOpa(LV_OPA_TRANSP, 0)
        ->SetStyleBorderWidth(0, 0)
        ->SetStylePadAll(0, 0)
        ->SetStylePadRow(6, 0)
        ->SetFlexFlow(LV_FLEX_FLOW_COLUMN)
        ->SetFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_clear_flag(bodyLayout_->GetObj(), LV_OBJ_FLAG_SCROLLABLE);

    networksLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    lv_label_set_text_static(networksLabel_->GetObj(), "Networks");
    networksLabel_->SetStyleTextColor(theme_.colors.contentFg, 0)
        ->SetStyleTextFont(theme_.fonts.contentSubheader, 0);

    networksList_ = std::make_unique<lvgl::widget::List>(bodyLayout_->GetObj());
    networksList_->SetSize(lv_pct(100), 130)
        ->SetStyleRadius(10, 0)
        ->SetStyleBgColor(theme_.colors.contentBg, 0)
        ->SetStyleBorderWidth(1, 0)
        ->SetStyleBorderColor(theme_.colors.shadow, 0);

    ssidLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    lv_label_set_text_static(ssidLabel_->GetObj(), "SSID");
    ssidLabel_->SetStyleTextColor(theme_.colors.contentFg, 0)
        ->SetStyleTextFont(theme_.fonts.contentSubheader, 0);

    ssidField_ = std::make_unique<lvgl::widget::TextArea>(bodyLayout_->GetObj());
    ssidField_->SetWidth(lv_pct(100));
    ssidField_->SetOneLine(true);
    lv_textarea_set_placeholder_text(ssidField_->GetObj(), "Select from list or type hidden SSID");
    ssidField_->SetStyleRadius(8, 0)
        ->SetStyleBorderWidth(1, 0)
        ->SetStyleBorderColor(theme_.colors.shadow, 0)
        ->SetStyleBgColor(theme_.colors.contentBg, 0)
        ->SetStyleTextColor(theme_.colors.contentFg, 0)
        ->SetStyleTextFont(theme_.fonts.contentSubheader, 0);
    lv_obj_add_event_cb(ssidField_->GetObj(), onTextAreaEvent, LV_EVENT_ALL, this);

    passwordLabel_ = std::make_unique<lvgl::widget::Label>(bodyLayout_->GetObj());
    lv_label_set_text_static(passwordLabel_->GetObj(), "Password");
    passwordLabel_->SetStyleTextColor(theme_.colors.contentFg, 0)
        ->SetStyleTextFont(theme_.fonts.contentSubheader, 0);

    passwordRow_ = std::make_unique<lvgl::widget::Object>(bodyLayout_->GetObj());
    passwordRow_->SetWidth(lv_pct(100))
        ->SetHeight(LV_SIZE_CONTENT)
        ->SetStyleBgOpa(LV_OPA_TRANSP, 0)
        ->SetStyleBorderWidth(0, 0)
        ->SetStylePadAll(0, 0)
        ->SetStylePadCollumn(6, 0)
        ->SetFlexFlow(LV_FLEX_FLOW_ROW)
        ->SetFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    passwordField_ = std::make_unique<lvgl::widget::TextArea>(passwordRow_->GetObj());
    lv_obj_set_flex_grow(passwordField_->GetObj(), 1);
    passwordField_->SetOneLine(true);
    passwordField_->SetPasswordMode(true);
    lv_textarea_set_placeholder_text(passwordField_->GetObj(), "WiFi password");
    passwordField_->SetStyleRadius(8, 0)
        ->SetStyleBorderWidth(1, 0)
        ->SetStyleBorderColor(theme_.colors.shadow, 0)
        ->SetStyleBgColor(theme_.colors.contentBg, 0)
        ->SetStyleTextColor(theme_.colors.contentFg, 0)
        ->SetStyleTextFont(theme_.fonts.contentSubheader, 0);
    lv_obj_add_event_cb(passwordField_->GetObj(), onTextAreaEvent, LV_EVENT_ALL, this);

    passwordToggleBtn_ = std::make_unique<lvgl::widget::Button>(passwordRow_->GetObj());
    passwordToggleBtn_->SetSize(52, 44)
        ->SetStyleRadius(8, 0)
        ->SetStyleBgColor(theme_.colors.buttonBg, 0)
        ->SetStyleBorderWidth(1, 0)
        ->SetStyleBorderColor(theme_.colors.shadow, 0)
        ->AddEventCbClicked(onPasswordToggleEvent, this);
    auto* toggleLabel = lv_label_create(passwordToggleBtn_->GetObj());
    lv_label_set_text_static(toggleLabel, LV_SYMBOL_EYE_OPEN);
    lv_obj_set_style_text_color(toggleLabel, theme_.colors.buttonFg, 0);
    lv_obj_set_style_text_font(toggleLabel, theme_.fonts.subheader, 0);
    lv_obj_center(toggleLabel);

    actionsRow_ = std::make_unique<lvgl::widget::Object>(bodyLayout_->GetObj());
    actionsRow_->SetWidth(lv_pct(100))
        ->SetHeight(84)
        ->SetStyleBgOpa(LV_OPA_TRANSP, 0)
        ->SetStyleBorderWidth(0, 0)
        ->SetStylePadAll(0, 0)
        ->SetStylePadCollumn(8, 0)
        ->SetFlexFlow(LV_FLEX_FLOW_ROW)
        ->SetFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    connectButton_ = std::make_unique<lvgl::widget::Button>(actionsRow_->GetObj());
    connectButton_->SetSize(150, 64)
        ->SetStyleRadius(10, 0)
        ->SetStylePadLeft(18, 0)
        ->SetStylePadRight(18, 0)
        ->SetStyleBgColor(theme_.colors.buttonBg, 0)
        ->SetStyleBorderWidth(0, 0)
        ->AddEventCbClicked(onConnectButtonEvent, this);

    auto* connectLabel = lv_label_create(connectButton_->GetObj());
    lv_label_set_text_static(connectLabel, "connect");
    lv_obj_set_style_text_color(connectLabel, theme_.colors.buttonFg, 0);
    lv_obj_set_style_text_font(connectLabel, theme_.fonts.subheader, 0);
    lv_obj_center(connectLabel);

    statusField_ = std::make_unique<lvgl::widget::TextArea>(actionsRow_->GetObj());
    lv_obj_set_flex_grow(statusField_->GetObj(), 1);
    statusField_->SetHeight(84);
    statusField_->SetOneLine(false);
    lv_textarea_set_text(statusField_->GetObj(), "idle");
    lv_textarea_set_cursor_click_pos(statusField_->GetObj(), false);
    lv_obj_clear_flag(statusField_->GetObj(), LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(statusField_->GetObj(), LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_state(statusField_->GetObj(), LV_STATE_DISABLED);
    statusField_->SetStyleRadius(10, 0)
        ->SetStyleBorderWidth(1, 0)
        ->SetStyleBorderColor(theme_.colors.shadow, 0)
        ->SetStyleBgColor(theme_.colors.contentBg, 0)
        ->SetStyleTextColor(theme_.colors.contentFg, 0)
        ->SetStyleTextFont(theme_.fonts.contentSubheader, 0)
        ->SetStylePadAll(8, 0);

    lv_obj_t* keyboard = lv_keyboard_create(parent);
    keyboard_.attach(keyboard);
    lv_obj_set_size(keyboard, lv_pct(100), lv_pct(50));
    lv_obj_align(keyboard, LV_ALIGN_TOP_MID, 0, 0);
    lv_keyboard_set_popovers(keyboard, true);
    lv_obj_set_style_pad_row(keyboard, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(keyboard, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(keyboard, theme_.fonts.subheader, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(keyboard, onKeyboardEvent, LV_EVENT_READY, this);
    lv_obj_add_event_cb(keyboard, onKeyboardEvent, LV_EVENT_CANCEL, this);

    // Emit characters on key release (keyup) rather than key press (keydown)
    lv_btnmatrix_set_btn_ctrl_all(keyboard, LV_BTNMATRIX_CTRL_CLICK_TRIG);

    setStatusText(F("Waiting for network list"));
}

void WifiConfigScreen::setNetworks(const BssList& networks)
{
    if(!networksList_ || !ssidField_) {
        return;
    }

    lv_obj_clean(networksList_->GetObj());

    for(size_t i = 0; i < networks.count(); ++i) {
        const BssInfo& net = networks[i];
        if(net.ssid.length() == 0) {
            continue;
        }

        lv_obj_t* btn = lv_list_add_btn(networksList_->GetObj(), nullptr, net.ssid.c_str());
        lv_obj_add_event_cb(btn, onNetworkButtonEvent, LV_EVENT_CLICKED, this);
    }

    if(networks.count() > 0 && lv_textarea_get_text(ssidField_->GetObj())[0] == '\0') {
        lv_textarea_set_text(ssidField_->GetObj(), networks[0].ssid.c_str());
    }
}

void WifiConfigScreen::setStatusText(const String& text)
{
    if(!statusField_) {
        return;
    }

    lv_textarea_set_text(statusField_->GetObj(), text.c_str());
}

void WifiConfigScreen::setOnConnectRequested(std::function<void(const String&, const String&)> callback)
{
    onConnectRequested_ = std::move(callback);
}

void WifiConfigScreen::showKeyboardFor(lv_obj_t* textArea)
{
    lv_obj_t* keyboard = keyboard_.get();
    if(keyboard == nullptr || textArea == nullptr) {
        return;
    }

    lv_keyboard_set_textarea(keyboard, textArea);
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

void WifiConfigScreen::hideKeyboard()
{
    lv_obj_t* keyboard = keyboard_.get();
    if(keyboard == nullptr) {
        return;
    }

    lv_keyboard_set_textarea(keyboard, nullptr);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
}

String WifiConfigScreen::selectedSsidFromList() const
{
    if(!ssidField_) {
        return String();
    }

    const char* text = lv_textarea_get_text(ssidField_->GetObj());
    return text ? String(text) : String();
}

void WifiConfigScreen::onNetworkButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<WifiConfigScreen*>(lv_event_get_user_data(event));
    if(self == nullptr || !self->networksList_ || !self->ssidField_) {
        return;
    }

    lv_obj_t* btn = lv_event_get_target(event);
    const char* ssid = lv_list_get_btn_text(self->networksList_->GetObj(), btn);
    if(ssid != nullptr && ssid[0] != '\0') {
        lv_textarea_set_text(self->ssidField_->GetObj(), ssid);
    }
}

void WifiConfigScreen::onConnectButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<WifiConfigScreen*>(lv_event_get_user_data(event));
    if(self == nullptr || !self->onConnectRequested_ || !self->passwordField_) {
        return;
    }

    const String ssid = self->selectedSsidFromList();
    const char* pw = lv_textarea_get_text(self->passwordField_->GetObj());
    const String password = pw ? String(pw) : String();

    if(ssid.length() == 0) {
        self->setStatusText(F("Please enter an SSID"));
        return;
    }

    String statusMsg = F("Connecting to ");
    statusMsg += ssid;
    self->setStatusText(statusMsg);
    self->hideKeyboard();
    self->onConnectRequested_(ssid, password);
}

void WifiConfigScreen::onTextAreaEvent(lv_event_t* event)
{
    auto* self = static_cast<WifiConfigScreen*>(lv_event_get_user_data(event));
    if(self == nullptr) {
        return;
    }

    const lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* target = lv_event_get_target(event);

    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        self->showKeyboardFor(target);
        return;
    }

    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        self->hideKeyboard();
    }
}

void WifiConfigScreen::onKeyboardEvent(lv_event_t* event)
{
    auto* self = static_cast<WifiConfigScreen*>(lv_event_get_user_data(event));
    if(self == nullptr) {
        return;
    }

    const lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        self->hideKeyboard();
    }
}

void WifiConfigScreen::onPasswordToggleEvent(lv_event_t* event)
{
    auto* self = static_cast<WifiConfigScreen*>(lv_event_get_user_data(event));
    if(self == nullptr || !self->passwordField_ || !self->passwordToggleBtn_) {
        return;
    }

    self->passwordVisible_ = !self->passwordVisible_;
    lv_textarea_set_password_mode(self->passwordField_->GetObj(), !self->passwordVisible_);

    lv_obj_t* btn = self->passwordToggleBtn_->GetObj();
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    if(label != nullptr) {
        lv_label_set_text_static(label, self->passwordVisible_ ? LV_SYMBOL_EYE_CLOSE : LV_SYMBOL_EYE_OPEN);
    }
}

} // namespace lightinator::ui::screens

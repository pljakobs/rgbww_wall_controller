#include "ui/screens/SettingsScreen.h"

#include <cstdio>

#include "ui/core/UiPolicy.h"

namespace lightinator::ui::screens {

SettingsScreen::SettingsScreen(const core::UiTheme& theme)
    : theme_(theme)
{
}

void SettingsScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void SettingsScreen::setOnSaveRequested(std::function<bool(int brightness, int timeout)> callback)
{
    onSaveRequested_ = std::move(callback);
}

void SettingsScreen::setOnBrightnessPreviewRequested(std::function<void(int brightness)> callback)
{
    onBrightnessPreviewRequested_ = std::move(callback);
}

void SettingsScreen::setOnOpenTouchCalibrationRequested(std::function<void()> callback)
{
    onOpenTouchCalibrationRequested_ = std::move(callback);
}

void SettingsScreen::setInitialValues(int brightness, int timeout)
{
    brightness_ = brightness;
    timeout_ = timeout;
    initialBrightness_ = brightness;
    initialTimeout_ = timeout;
}

void SettingsScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text = "Settings";
    header.showClose = true;
    header.height = theme_.headerHeight;
    header.color = theme_.colors.headerBg;
    header.font = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
    decorated_->setOnCloseRequested([this]() {
        if (onCloseRequested_) {
            onCloseRequested_();
        }
    });

    decorated_->mount(parent);
    setRoot(decorated_->root());

    lv_obj_t* body = decorated_->bodySlot();
    lv_obj_set_style_bg_color(body, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(body, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(body, 12, 0);
    lv_obj_set_style_pad_row(body, 10, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* brightnessTitle = lv_label_create(body);
    lv_label_set_text_static(brightnessTitle, "Backlight brightness");
    lv_obj_set_style_text_font(brightnessTitle, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(brightnessTitle, theme_.colors.contentFg, 0);

    brightnessValueLabel_ = lv_label_create(body);
    lv_obj_set_style_text_font(brightnessValueLabel_, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(brightnessValueLabel_, theme_.colors.contentFg, 0);

    brightnessSlider_ = lv_slider_create(body);
    lv_obj_set_width(brightnessSlider_, lv_pct(100));
    lv_slider_set_range(brightnessSlider_, 0, 100);
    lv_slider_set_value(brightnessSlider_, brightness_, LV_ANIM_OFF);
    lv_obj_add_event_cb(brightnessSlider_, onSliderEvent, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_add_event_cb(brightnessSlider_, onSliderEvent, LV_EVENT_RELEASED, this);

    lv_obj_t* timeoutTitle = lv_label_create(body);
    lv_label_set_text_static(timeoutTitle, "Backlight timeout");
    lv_obj_set_style_text_font(timeoutTitle, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(timeoutTitle, theme_.colors.contentFg, 0);

    timeoutValueLabel_ = lv_label_create(body);
    lv_obj_set_style_text_font(timeoutValueLabel_, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(timeoutValueLabel_, theme_.colors.contentFg, 0);

    timeoutSlider_ = lv_slider_create(body);
    lv_obj_set_width(timeoutSlider_, lv_pct(100));
    lv_slider_set_range(timeoutSlider_, core::policy::kTimeoutSliderMinStep, core::policy::kTimeoutSliderMaxStep);
    lv_slider_set_value(timeoutSlider_, timeoutSliderValueForSeconds(timeout_), LV_ANIM_OFF);
    lv_obj_add_event_cb(timeoutSlider_, onSliderEvent, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_add_event_cb(timeoutSlider_, onSliderEvent, LV_EVENT_RELEASED, this);

    lv_obj_t* calibrationButton = lv_btn_create(body);
    lv_obj_set_size(calibrationButton, lv_pct(100), 44);
    lv_obj_set_style_bg_color(calibrationButton, theme_.colors.buttonBg, 0);
    lv_obj_set_style_text_color(calibrationButton, theme_.colors.buttonFg, 0);
    lv_obj_set_style_border_width(calibrationButton, 0, 0);
    lv_obj_add_event_cb(calibrationButton, onOpenCalibrationEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* calibrationLabel = lv_label_create(calibrationButton);
    lv_label_set_text_static(calibrationLabel, "Touch Calibration");
    lv_obj_set_style_text_font(calibrationLabel, theme_.fonts.contentSubheader, 0);
    lv_obj_center(calibrationLabel);

    statusLabel_ = lv_label_create(body);
    lv_label_set_text_static(statusLabel_, "");
    lv_obj_set_style_text_font(statusLabel_, theme_.fonts.content, 0);
    lv_obj_set_style_text_color(statusLabel_, theme_.colors.contentFg, 0);
    lv_label_set_long_mode(statusLabel_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(statusLabel_, lv_pct(100));

    refreshValueLabels();

    footer_ = lv_obj_create(body);
    lv_obj_set_width(footer_, lv_pct(100));
    lv_obj_set_height(footer_, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(footer_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(footer_, 0, 0);
    lv_obj_set_style_pad_all(footer_, 0, 0);
    lv_obj_set_style_pad_column(footer_, 10, 0);
    lv_obj_set_flex_flow(footer_, LV_FLEX_FLOW_ROW);

    lv_obj_t* okButton = lv_btn_create(footer_);
    lv_obj_set_size(okButton, 120, 44);
    lv_obj_set_style_bg_color(okButton, theme_.colors.buttonBg, 0);
    lv_obj_set_style_text_color(okButton, theme_.colors.buttonFg, 0);
    lv_obj_set_style_border_width(okButton, 0, 0);
    lv_obj_add_event_cb(okButton, onOkButtonEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* okLabel = lv_label_create(okButton);
    lv_label_set_text_static(okLabel, "OK");
    lv_obj_set_style_text_font(okLabel, theme_.fonts.contentSubheader, 0);
    lv_obj_center(okLabel);

    lv_obj_t* cancelButton = lv_btn_create(footer_);
    lv_obj_set_size(cancelButton, 120, 44);
    lv_obj_set_style_bg_color(cancelButton, theme_.colors.buttonBg, 0);
    lv_obj_set_style_text_color(cancelButton, theme_.colors.buttonFg, 0);
    lv_obj_set_style_border_width(cancelButton, 0, 0);
    lv_obj_add_event_cb(cancelButton, onCancelButtonEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* cancelLabel = lv_label_create(cancelButton);
    lv_label_set_text_static(cancelLabel, "Cancel");
    lv_obj_set_style_text_font(cancelLabel, theme_.fonts.contentSubheader, 0);
    lv_obj_center(cancelLabel);
}

void SettingsScreen::onOkButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    self->emitBrightnessPreview(true);
    self->saveSettings(true);
}

void SettingsScreen::onCancelButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    self->brightness_ = self->initialBrightness_;
    self->timeout_ = self->initialTimeout_;
    self->emitBrightnessPreview(true);

    if (self->onCloseRequested_) {
        self->onCloseRequested_();
    }
}

void SettingsScreen::onSliderEvent(lv_event_t* event)
{
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    const lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* target = lv_event_get_target(event);
    if (target == self->brightnessSlider_) {
        self->brightness_ = lv_slider_get_value(self->brightnessSlider_);
        self->emitBrightnessPreview(code == LV_EVENT_RELEASED);
    } else if (target == self->timeoutSlider_) {
        self->timeout_ = timeoutSecondsForSliderValue(lv_slider_get_value(self->timeoutSlider_));
    }

    self->refreshValueLabels();
}

void SettingsScreen::onOpenCalibrationEvent(lv_event_t* event)
{
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(event));
    if (self == nullptr || !self->onOpenTouchCalibrationRequested_) {
        return;
    }
    self->onOpenTouchCalibrationRequested_();
}

void SettingsScreen::refreshValueLabels()
{
    if (brightnessValueLabel_ != nullptr) {
        char brightnessText[32] = {0};
        std::snprintf(brightnessText, sizeof(brightnessText), "%d%%", brightness_);
        lv_label_set_text(brightnessValueLabel_, brightnessText);
    }

    if (timeoutValueLabel_ != nullptr) {
        char timeoutText[48] = {0};
        if (timeout_ < 0) {
            std::snprintf(timeoutText, sizeof(timeoutText), "Never");
        } else {
            std::snprintf(timeoutText, sizeof(timeoutText), "%d s", timeout_);
        }
        lv_label_set_text(timeoutValueLabel_, timeoutText);
    }
}

void SettingsScreen::emitBrightnessPreview(bool force)
{
    if (!onBrightnessPreviewRequested_) {
        return;
    }

    const uint32_t now = lv_tick_get();
    if (!force && lastPreviewBrightness_ == brightness_ && (now - lastBrightnessPreviewTick_) < core::policy::kBrightnessPreviewIntervalMs) {
        return;
    }
    if (!force && lastPreviewBrightness_ != brightness_ && (now - lastBrightnessPreviewTick_) < core::policy::kBrightnessPreviewIntervalMs) {
        return;
    }

    lastBrightnessPreviewTick_ = now;
    lastPreviewBrightness_ = brightness_;
    onBrightnessPreviewRequested_(brightness_);
}

bool SettingsScreen::saveSettings(bool closeOnSuccess)
{
    bool saved = true;
    if (onSaveRequested_) {
        saved = onSaveRequested_(brightness_, timeout_);
    }

    if (statusLabel_ != nullptr) {
        lv_label_set_text_static(statusLabel_, saved ? "Settings saved" : "Failed to save settings");
    }

    if (saved && closeOnSuccess && onCloseRequested_) {
        onCloseRequested_();
    }
    return saved;
}

int SettingsScreen::timeoutSliderValueForSeconds(int timeout)
{
    if (timeout < 0) {
        return core::policy::kTimeoutSliderMaxStep;
    }
    int clamped = timeout;
    if (clamped < core::policy::kTimeoutStepSeconds) {
        clamped = core::policy::kTimeoutStepSeconds;
    } else if (clamped > core::policy::kTimeoutMaxSeconds) {
        clamped = core::policy::kTimeoutMaxSeconds;
    }
    return (clamped + (core::policy::kTimeoutStepSeconds - 1)) / core::policy::kTimeoutStepSeconds;
}

int SettingsScreen::timeoutSecondsForSliderValue(int sliderValue)
{
    if (sliderValue >= core::policy::kTimeoutSliderMaxStep) {
        return -1;
    }
    int clamped = sliderValue;
    if (clamped < core::policy::kTimeoutSliderMinStep) {
        clamped = core::policy::kTimeoutSliderMinStep;
    }
    return clamped * core::policy::kTimeoutStepSeconds;
}

} // namespace lightinator::ui::screens

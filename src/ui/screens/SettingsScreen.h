#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include <lvgl.h>

#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

class SettingsScreen : public core::Screen {
public:
    explicit SettingsScreen(const core::UiTheme& theme);
    ~SettingsScreen() override;

    void mount(lv_obj_t* parent) override;
    void setOnCloseRequested(std::function<void()> callback);
    void setOnSaveRequested(std::function<bool(int brightness, int timeout)> callback);
    void setOnBrightnessPreviewRequested(std::function<void(int brightness)> callback);
    void setOnOpenTouchCalibrationRequested(std::function<void()> callback);
    void setInitialValues(int brightness, int timeout);

private:
    static void onOkButtonEvent(lv_event_t* event);
    static void onCancelButtonEvent(lv_event_t* event);
    static void onSliderEvent(lv_event_t* event);
    static void onDeferredSaveTimer(lv_timer_t* timer);
    static void onOpenCalibrationEvent(lv_event_t* event);

    void emitBrightnessPreview(bool force);
    void scheduleDeferredSave();
    bool saveSettings(bool closeOnSuccess);
    void refreshValueLabels();
    static int timeoutSliderValueForSeconds(int timeout);
    static int timeoutSecondsForSliderValue(int sliderValue);

    core::UiTheme theme_;
    std::function<void()> onCloseRequested_;

    std::function<bool(int brightness, int timeout)> onSaveRequested_;
    std::function<void(int brightness)> onBrightnessPreviewRequested_;
    std::function<void()> onOpenTouchCalibrationRequested_;

    int brightness_ = 80;
    int timeout_ = 30;
    uint32_t lastBrightnessPreviewTick_ = 0;
    int lastPreviewBrightness_ = -1;
    lv_timer_t* deferredSaveTimer_ = nullptr;

    lv_obj_t* brightnessSlider_ = nullptr;
    lv_obj_t* timeoutSlider_ = nullptr;
    lv_obj_t* brightnessValueLabel_ = nullptr;
    lv_obj_t* timeoutValueLabel_ = nullptr;
    lv_obj_t* statusLabel_ = nullptr;
    lv_obj_t* footer_ = nullptr;

    std::unique_ptr<DecoratedScreen> decorated_;
};

} // namespace lightinator::ui::screens

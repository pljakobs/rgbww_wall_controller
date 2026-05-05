#pragma once

#include <functional>
#include <memory>
#include <array>

#include <lvgl.h>

#include "ui/core/TouchCalibrationData.h"
#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

class TouchCalibrationScreen : public core::Screen {
public:
    explicit TouchCalibrationScreen(const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;
    void setOnCloseRequested(std::function<void()> callback);
    void setOnSaveRequested(std::function<bool(const core::TouchCalibrationCapture& capture)> callback);

private:
    struct CalibrationPoint {
        int x;
        int y;
    };

    static void onCaptureTapEvent(lv_event_t* event);

    void updateCrossStates();
    core::TouchCalibrationCapture buildCalibrationCapture() const;
    static CalibrationPoint targetPointForIndex(int index, int width, int height);
    static bool isPointInsideTarget(const CalibrationPoint& tap,
                                    const CalibrationPoint& target,
                                    int tolerance);

    core::UiTheme theme_;
    std::function<void()> onCloseRequested_;

    std::function<bool(const core::TouchCalibrationCapture& capture)> onSaveRequested_;

    int captureIndex_ = 0;
    bool touchCapturedForCurrentPress_ = false;
    std::array<CalibrationPoint, 5> measuredPoints_ = {};
    std::array<lv_obj_t*, 5> crossLabels_ = {};
    std::array<lv_obj_t*, 5> detectedCrossLabels_ = {};

    lv_obj_t* body_ = nullptr;
    lv_obj_t* instructionLabel_ = nullptr;

    std::unique_ptr<DecoratedScreen> decorated_;
};

} // namespace lightinator::ui::screens

#include "ui/screens/TouchCalibrationScreen.h"

#include <cstdlib>

#include "ui/core/UiPolicy.h"

extern "C" {
#include "touch_driver_gt911/touch_driver_gt911.h"
}

namespace lightinator::ui::screens {

TouchCalibrationScreen::TouchCalibrationScreen(const core::UiTheme& theme)
    : theme_(theme)
{
}

void TouchCalibrationScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void TouchCalibrationScreen::setOnSaveRequested(std::function<bool(const core::TouchCalibrationCapture& capture)> callback)
{
    onSaveRequested_ = std::move(callback);
}

void TouchCalibrationScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text = "Touch Calibration";
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

    body_ = decorated_->bodySlot();
    lv_obj_set_style_bg_color(body_, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(body_, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(body_, 0, 0);
    lv_obj_clear_flag(body_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(body_, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_add_event_cb(body_, onCaptureTapEvent, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(body_, onCaptureTapEvent, LV_EVENT_PRESSING, this);
    lv_obj_add_event_cb(body_, onCaptureTapEvent, LV_EVENT_RELEASED, this);

    lv_obj_update_layout(decorated_->root());
    const lv_coord_t width = lv_obj_get_content_width(body_);
    const lv_coord_t height = lv_obj_get_content_height(body_);
    for (int i = 0; i < 5; i++) {
        lv_obj_t* cross = lv_label_create(body_);
        crossLabels_[i] = cross;
        lv_label_set_text_static(cross, "+");
        lv_obj_set_style_text_font(cross, theme_.fonts.header, 0);
        lv_obj_clear_flag(cross, LV_OBJ_FLAG_CLICKABLE);
        const CalibrationPoint target = targetPointForIndex(i, width, height);
        lv_obj_set_pos(cross, target.x - 10, target.y - 18);

        lv_obj_t* detectedCross = lv_label_create(body_);
        detectedCrossLabels_[i] = detectedCross;
        lv_label_set_text_static(detectedCross, "+");
        lv_obj_set_style_text_font(detectedCross, theme_.fonts.header, 0);
        lv_obj_set_style_text_color(detectedCross, theme_.colors.contentFg, 0);
        lv_obj_set_style_text_opa(detectedCross, LV_OPA_40, 0);
        lv_obj_add_flag(detectedCross, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(detectedCross, LV_OBJ_FLAG_CLICKABLE);
    }

    instructionLabel_ = lv_label_create(body_);
    lv_label_set_text_static(instructionLabel_, "touch the colored cross");
    lv_obj_set_style_text_font(instructionLabel_, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(instructionLabel_, theme_.colors.contentFg, 0);
    lv_obj_clear_flag(instructionLabel_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align_to(instructionLabel_, crossLabels_[4], LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    updateCrossStates();
}

void TouchCalibrationScreen::onCaptureTapEvent(lv_event_t* event)
{
    auto* self = static_cast<TouchCalibrationScreen*>(lv_event_get_user_data(event));
    if (self == nullptr || self->captureIndex_ >= 5) {
        return;
    }

    const lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_RELEASED) {
        self->touchCapturedForCurrentPress_ = false;
        return;
    }
    if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING) {
        return;
    }
    if (self->touchCapturedForCurrentPress_) {
        return;
    }

    lv_indev_t* indev = lv_event_get_indev(event);
    if (indev == nullptr) {
        indev = lv_indev_get_act();
    }
    if (indev == nullptr) {
        return;
    }
    lv_point_t point = {};
    lv_indev_get_point(indev, &point);

    lv_area_t area = {};
    lv_obj_get_coords(self->body_, &area);
    const CalibrationPoint localTapPoint = {
        point.x - area.x1,
        point.y - area.y1,
    };
    const lv_coord_t width = lv_obj_get_content_width(self->body_);
    const lv_coord_t height = lv_obj_get_content_height(self->body_);
    const CalibrationPoint target = targetPointForIndex(self->captureIndex_, width, height);
    if (!isPointInsideTarget(localTapPoint, target, core::policy::kTouchCalibrationHitTolerancePx)) {
        return;
    }

    int16_t rawX = point.x;
    int16_t rawY = point.y;
    touch_driver_gt911_get_last_raw(&rawX, &rawY);
    self->measuredPoints_[self->captureIndex_] = {rawX, rawY};
    self->detectedPoints_[self->captureIndex_] = localTapPoint;
    self->touchCapturedForCurrentPress_ = true;
    self->captureIndex_++;
    self->updateCrossStates();

    if (self->captureIndex_ < 5) {
        return;
    }

    bool saved = true;
    if (self->onSaveRequested_) {
        const core::TouchCalibrationCapture capture = self->buildCalibrationCapture();
        saved = self->onSaveRequested_(capture);
    }
    if (!saved) {
        if (self->instructionLabel_ != nullptr) {
            lv_label_set_text_static(self->instructionLabel_, "save failed - touch colored cross");
        }
        self->captureIndex_ = 0;
        self->updateCrossStates();
        return;
    }

    if (self->onCloseRequested_) {
        self->onCloseRequested_();
    }
}

void TouchCalibrationScreen::updateCrossStates()
{
    if (instructionLabel_ == nullptr) {
        return;
    }

    for (int i = 0; i < 5; i++) {
        lv_obj_t* cross = crossLabels_[i];
        if (cross == nullptr) {
            continue;
        }

        lv_color_t color = theme_.colors.contentFg;
        if (i == captureIndex_) {
            color = theme_.colors.dangerBg;
        }
        lv_obj_set_style_text_color(cross, color, 0);

        lv_obj_t* detectedCross = detectedCrossLabels_[i];
        if (detectedCross != nullptr) {
            if (i < captureIndex_) {
                lv_obj_clear_flag(detectedCross, LV_OBJ_FLAG_HIDDEN);
                const CalibrationPoint detected = detectedPoints_[i];
                lv_obj_set_pos(detectedCross,
                               detected.x - 10,
                               detected.y - 18);
            } else {
                lv_obj_add_flag(detectedCross, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    lv_label_set_text_static(instructionLabel_, "touch the colored cross");
}

core::TouchCalibrationCapture TouchCalibrationScreen::buildCalibrationCapture() const
{
    core::TouchCalibrationCapture capture = {};
    capture.screenWidth = lv_obj_get_width(decorated_->root());
    capture.screenHeight = lv_obj_get_height(decorated_->root());

    lv_area_t area = {};
    lv_obj_get_coords(body_, &area);
    const lv_coord_t width = lv_obj_get_content_width(body_);
    const lv_coord_t height = lv_obj_get_content_height(body_);

    for (int i = 0; i < 5; i++) {
        const CalibrationPoint target = targetPointForIndex(i, width, height);
        const CalibrationPoint raw = measuredPoints_[i];
        capture.points[i].referenceX = area.x1 + target.x;
        capture.points[i].referenceY = area.y1 + target.y;
        capture.points[i].rawX = raw.x;
        capture.points[i].rawY = raw.y;
    }

    return capture;
}

TouchCalibrationScreen::CalibrationPoint TouchCalibrationScreen::targetPointForIndex(int index,
                                                                                      int width,
                                                                                      int height)
{
    const int margin = core::policy::kTouchCalibrationTargetMarginPx;
    switch (index) {
    case 0:
        return {margin, margin};
    case 1:
        return {width - margin, margin};
    case 2:
        return {width - margin, height - margin};
    case 3:
        return {margin, height - margin};
    case 4:
    default:
        return {width / 2, height / 2};
    }
}

bool TouchCalibrationScreen::isPointInsideTarget(const CalibrationPoint& tap,
                                                 const CalibrationPoint& target,
                                                 int tolerance)
{
    const int dx = tap.x - target.x;
    const int dy = tap.y - target.y;
    const int absDx = (dx < 0) ? -dx : dx;
    const int absDy = (dy < 0) ? -dy : dy;
    return (absDx <= tolerance) && (absDy <= tolerance);
}

} // namespace lightinator::ui::screens

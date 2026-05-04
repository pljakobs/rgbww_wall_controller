#include "ui/widgets/HsvColorPicker.h"

#include <algorithm>
#include <cstdint>

namespace lightinator::ui::widgets {
namespace {

constexpr lv_coord_t kSvSize = 260;
constexpr lv_coord_t kMarkerSize = 10;
constexpr uint32_t kHueGradientStep = 10 * core::kHueScale;
constexpr uint32_t kHueEmitStep = 5 * core::kHueScale;

lv_coord_t clampCoord(lv_coord_t value, lv_coord_t min, lv_coord_t max)
{
    return std::min(max, std::max(min, value));
}

uint32_t hueDistance(uint32_t a, uint32_t b)
{
    const uint32_t fullTurn = 360 * core::kHueScale;
    const uint32_t direct = a > b ? (a - b) : (b - a);
    return std::min<uint32_t>(direct, fullTurn - direct);
}

} // namespace

void HsvColorPicker::mount(lv_obj_t* parent)
{
    color_ = core::clampHsv(color_);
    svSize_ = kSvSize;

    rootObj_ = std::make_unique<lvgl::widget::Object>(parent);
    setRoot(rootObj_->GetObj());
    rootObj_->SetSize(lv_pct(100), lv_pct(100))
        ->SetStyleBgOpa(LV_OPA_TRANSP, 0)
        ->SetStyleBorderWidth(0, 0)
        ->SetStylePadAll(0, 0)
        ->SetStylePadRow(14, 0)
        ->SetFlexFlow(LV_FLEX_FLOW_COLUMN)
        ->SetFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    svCanvas_ = std::make_unique<lvgl::widget::Canvas>(rootObj_->GetObj());
    lv_obj_t* sv = svCanvas_->GetObj();
    svCanvas_->SetSize(svSize_, svSize_)
        ->SetStyleBorderWidth(1, 0)
        ->SetStyleBorderColor(lv_color_hex(0x1C1C1C), 0)
        ->SetStyleRadius(0, 0)
        ->SetStyleClipCorner(true, 0);

    svBuffer_.assign(static_cast<size_t>(svSize_ * svSize_), lv_color_black());
    svCanvas_->SetBuffer(svBuffer_.data(), svSize_, svSize_, LV_IMG_CF_TRUE_COLOR);
    lv_obj_add_flag(sv, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_clear_flag(sv, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(sv, onSvCanvasEvent, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(sv, onSvCanvasEvent, LV_EVENT_PRESSING, this);
    lv_obj_add_event_cb(sv, onSvCanvasEvent, LV_EVENT_RELEASED, this);
    lv_obj_add_event_cb(sv, onSvCanvasEvent, LV_EVENT_CLICKED, this);

    markerCanvas_ = std::make_unique<lvgl::widget::Canvas>(sv);
    markerCanvas_->SetSize(kMarkerSize, kMarkerSize)
        ->ClearFlag(LV_OBJ_FLAG_CLICKABLE)
        ->SetStyleRadius(0, 0)
        ->SetStyleBgOpa(LV_OPA_TRANSP, 0)
        ->SetStyleBorderWidth(1, 0);
    lv_obj_move_foreground(markerCanvas_->GetObj());

    hueSlider_ = std::make_unique<lvgl::widget::Slider>(rootObj_->GetObj());
    hueSlider_->SetWidth(svSize_);
    hueSlider_->SetRange(0, 359);
    hueSlider_->SetValue(core::toLvHue(color_.h), LV_ANIM_OFF);
    hueSlider_->SetStylePadVer(10, LV_PART_MAIN);
    hueSlider_->SetStyleSize(32, LV_PART_KNOB);
    hueSlider_->AddEventCbValueChanged(onHueSliderEvent, this);
    lv_obj_add_event_cb(hueSlider_->GetObj(), onHueSliderEvent, LV_EVENT_RELEASED, this);
    gradientHue_ = color_.h;
    emittedHue_ = color_.h;

    okButton_ = std::make_unique<lvgl::widget::Button>(rootObj_->GetObj());
    okButton_->SetSize(120, 48)
        ->SetAlign(LV_ALIGN_BOTTOM_RIGHT, 0, 10)
        ->SetStyleBgColor(lv_color_hex(0x2A7A3B), 0)
        ->SetStyleBorderWidth(0, 0)
        ->AddEventCbClicked(onOkButtonEvent, this);
    lv_obj_t* ok = okButton_->GetObj();

    auto* okLabel = lv_label_create(ok);
    lv_label_set_text_static(okLabel, "OK");
    lv_obj_center(okLabel);

    refreshGradient();
    refreshMarker();
}

void HsvColorPicker::setValue(core::HsvColor value)
{
    color_ = core::clampHsv(value);
    if (hueSlider_) {
        lv_slider_set_value(hueSlider_->GetObj(), core::toLvHue(color_.h), LV_ANIM_OFF);
    }
    gradientHue_ = color_.h;
    emittedHue_ = color_.h;
    markerFromTouch_ = false;
    if (svCanvas_) {
        refreshGradient();
        refreshMarker();
    }
}

core::HsvColor HsvColorPicker::value() const
{
    return color_;
}

void HsvColorPicker::setOnColorChanged(std::function<void(const core::HsvColor&)> callback)
{
    onColorChanged_ = std::move(callback);
}

void HsvColorPicker::setOnColorCommitted(std::function<void(const core::HsvColor&)> callback)
{
    onColorCommitted_ = std::move(callback);
}

void HsvColorPicker::onHueSliderEvent(lv_event_t* event)
{
    const lv_event_code_t code = lv_event_get_code(event);
    if (code != LV_EVENT_VALUE_CHANGED && code != LV_EVENT_RELEASED) {
        return;
    }

    auto* self = static_cast<HsvColorPicker*>(lv_event_get_user_data(event));
    if (self == nullptr || self->hueSlider_ == nullptr) {
        return;
    }

    const uint16_t newHueLv = static_cast<uint16_t>(lv_slider_get_value(self->hueSlider_->GetObj()));
    const uint32_t newHue = core::fromLvHue(newHueLv);
    if (newHue == self->color_.h && code == LV_EVENT_VALUE_CHANGED) {
        return;
    }

    self->color_.h = newHue;
    if (code == LV_EVENT_RELEASED || hueDistance(self->gradientHue_, newHue) >= kHueGradientStep) {
        self->refreshGradient();
        self->gradientHue_ = newHue;
    }
    if (code == LV_EVENT_RELEASED || hueDistance(self->emittedHue_, newHue) >= kHueEmitStep) {
        self->emitChanged();
        self->emittedHue_ = newHue;
    }
}

void HsvColorPicker::onSvCanvasEvent(lv_event_t* event)
{
    const lv_event_code_t code = lv_event_get_code(event);
    if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING && code != LV_EVENT_RELEASED
        && code != LV_EVENT_CLICKED) {
        return;
    }

    auto* self = static_cast<HsvColorPicker*>(lv_event_get_user_data(event));
    if (self == nullptr || self->svCanvas_ == nullptr) {
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
    lv_obj_get_coords(self->svCanvas_->GetObj(), &area);
    const lv_coord_t maxPos = self->svSize_ > 0 ? (self->svSize_ - 1) : 0;
    const lv_coord_t localX = clampCoord(point.x - area.x1, 0, maxPos);
    const lv_coord_t localY = clampCoord(point.y - area.y1, 0, maxPos);

    self->markerX_ = localX;
    self->markerY_ = localY;
    self->markerFromTouch_ = true;
    self->setFromCanvasPoint(localX, localY);
    self->refreshMarker();
    self->emitChanged();
}

void HsvColorPicker::onOkButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<HsvColorPicker*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    if (self->onColorCommitted_) {
        self->onColorCommitted_(self->color_);
    }
}

void HsvColorPicker::refreshGradient()
{
    if (svBuffer_.empty() || svCanvas_ == nullptr) {
        return;
    }

    const lv_coord_t denom = std::max<lv_coord_t>(1, svSize_ - 1);
    for (lv_coord_t y = 0; y < svSize_; ++y) {
        const uint8_t value = static_cast<uint8_t>(100 - ((y * 100) / denom));
        for (lv_coord_t x = 0; x < svSize_; ++x) {
            const uint8_t sat = static_cast<uint8_t>((x * 100) / denom);
            const size_t index = static_cast<size_t>(y * svSize_ + x);
            svBuffer_[index] = lv_color_hsv_to_rgb(core::toLvHue(color_.h), sat, value);
        }
    }

    svCanvas_->Invalidate();
}

void HsvColorPicker::refreshMarker()
{
    if (markerCanvas_ == nullptr) {
        return;
    }

    const bool useWhite = color_.v < (25 * core::kPercentScale);
    markerCanvas_->SetStyleBorderColor(useWhite ? lv_color_white() : lv_color_black(), 0);

    const lv_coord_t maxPos = std::max<lv_coord_t>(0, svSize_ - 1);
    const lv_coord_t markerX = markerFromTouch_
        ? markerX_
        : static_cast<lv_coord_t>((color_.s * maxPos) / core::kPercentMax);
    const lv_coord_t markerY = markerFromTouch_
        ? markerY_
        : static_cast<lv_coord_t>(((core::kPercentMax - color_.v) * maxPos) / core::kPercentMax);
    markerCanvas_->SetPos(
        clampCoord(markerX - (kMarkerSize / 2), 0, std::max<lv_coord_t>(0, svSize_ - kMarkerSize)),
        clampCoord(markerY - (kMarkerSize / 2), 0, std::max<lv_coord_t>(0, svSize_ - kMarkerSize)));
}

void HsvColorPicker::setFromCanvasPoint(lv_coord_t x, lv_coord_t y)
{
    const lv_coord_t denom = std::max<lv_coord_t>(1, svSize_ - 1);
    color_.s = static_cast<uint32_t>((static_cast<uint64_t>(x) * core::kPercentMax) / denom);
    color_.v = static_cast<uint32_t>((static_cast<uint64_t>(denom - y) * core::kPercentMax) / denom);
}

void HsvColorPicker::emitChanged()
{
    if (onColorChanged_) {
        onColorChanged_(color_);
    }
}

} // namespace lightinator::ui::widgets

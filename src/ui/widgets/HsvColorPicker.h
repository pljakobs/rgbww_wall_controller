#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <lvgl.h>
#include <lvglCpp.h>

#include "ui/core/CompositeWidget.h"
#include "ui/core/HsvColor.h"

namespace lightinator::ui::widgets {

class HsvColorPicker : public core::CompositeWidget {
public:
    void mount(lv_obj_t* parent) override;

    void setValue(core::HsvColor value);
    core::HsvColor value() const;

    void setOnColorChanged(std::function<void(const core::HsvColor&)> callback);
    void setOnColorCommitted(std::function<void(const core::HsvColor&)> callback);

private:
    static void onLayoutEvent(lv_event_t* event);
    static void onHueSliderEvent(lv_event_t* event);
    static void onSvCanvasEvent(lv_event_t* event);
    static void onOkButtonEvent(lv_event_t* event);

    void updateLayout();
    void refreshGradient(uint8_t cellSize = 1);
    void refreshHueStrip();
    void refreshMarker();
    void setFromCanvasPoint(lv_coord_t x, lv_coord_t y);
    void emitChanged();

    core::HsvColor color_;

    std::function<void(const core::HsvColor&)> onColorChanged_;
    std::function<void(const core::HsvColor&)> onColorCommitted_;

    std::unique_ptr<lvgl::widget::Object> rootObj_;
    std::unique_ptr<lvgl::widget::Object> rightCol_;
    std::unique_ptr<lvgl::widget::Object> hueArea_;
    std::unique_ptr<lvgl::widget::Canvas> svCanvas_;
    std::unique_ptr<lvgl::widget::Canvas> hueStripCanvas_;
    std::unique_ptr<lvgl::widget::Canvas> markerCanvas_;
    std::unique_ptr<lvgl::widget::Slider> hueSlider_;
    std::unique_ptr<lvgl::widget::Button> okButton_;

    std::vector<lv_color_t> svBuffer_;
    std::vector<lv_color_t> hueStripBuffer_;
    lv_coord_t svSize_ = 0;
    lv_coord_t hueStripHeight_ = 0;
    lv_coord_t markerX_ = 0;
    lv_coord_t markerY_ = 0;
    bool markerFromTouch_ = false;
    uint32_t gradientHue_ = 0;
    uint32_t emittedHue_ = 0;
};

} // namespace lightinator::ui::widgets

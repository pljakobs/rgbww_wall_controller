#pragma once

#include <functional>
#include <memory>

#include <lvgl.h>

#include "ui/core/HsvColor.h"
#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"
#include "ui/widgets/HsvColorPicker.h"

namespace lightinator::ui::screens {

class ColorPickerScreen : public core::Screen {
public:
    explicit ColorPickerScreen(core::HsvColor initialColor, const core::UiTheme& theme = core::nordicDarkTheme());

    void mount(lv_obj_t* parent) override;

    void setOnCloseRequested(std::function<void()> callback);
    void setOnColorChanged(std::function<void(const core::HsvColor&)> callback);

private:
    core::HsvColor initialColor_;
    core::UiTheme theme_;

    std::function<void()> onCloseRequested_;
    std::function<void(const core::HsvColor&)> onColorChanged_;
    core::HsvColor currentColor_;

    std::unique_ptr<DecoratedScreen> decorated_;
};

} // namespace lightinator::ui::screens

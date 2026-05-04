#pragma once

#include <functional>
#include <memory>

#include <lvgl.h>

#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

class MenuTestScreen : public core::Screen {
public:
    explicit MenuTestScreen(const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;
    void setOnCloseRequested(std::function<void()> callback);

private:
    core::UiTheme theme_;
    std::function<void()> onCloseRequested_;

    std::unique_ptr<DecoratedScreen> decorated_;
};

} // namespace lightinator::ui::screens

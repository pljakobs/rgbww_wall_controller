#pragma once

#include <functional>
#include <memory>

#include <lvgl.h>
#include <lvglCpp.h>

#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

class ThemePreviewScreen : public core::Screen {
public:
    explicit ThemePreviewScreen(const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;

    void setOnCloseRequested(std::function<void()> callback);

private:
    void buildColorsTab(lv_obj_t* tab);
    void buildFontsTab(lv_obj_t* tab);

    void addColorRow(lv_obj_t* parent, const char* name, lv_color_t color);
    void addFontRow(lv_obj_t* parent, const char* name, const lv_font_t* font);

    core::UiTheme theme_;
    std::function<void()> onCloseRequested_;

    std::unique_ptr<DecoratedScreen> decorated_;
};

} // namespace lightinator::ui::screens

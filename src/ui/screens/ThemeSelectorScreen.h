#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <lvgl.h>
#include <WString.h>

#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

/**
 * ThemeSelectorScreen — browse and select from available themes.
 *
 * Shows a dropdown of all available themes together with a color-swatch
 * preview of the selection. Provides two actions:
 *   Apply  — applies the selected theme without saving (live preview).
 *   Edit   — opens ThemeEditorScreen pre-loaded with the selection and a
 *             suggested save-name based on whether the theme is built-in
 *             or user-configDB.
 */
class ThemeSelectorScreen : public core::Screen {
public:
    explicit ThemeSelectorScreen(const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;

    void setOnCloseRequested(std::function<void()> callback);
    void setOnThemeListRequested(std::function<std::vector<core::UiTheme>()> callback);
    void setOnThemeApplyRequested(std::function<void(const core::UiTheme&)> callback);
    /// Called with (themeToEdit, suggestedName) when the user taps Edit.
    void setOnThemeEditRequested(
        std::function<void(const core::UiTheme&, const String&)> callback);

private:
    static void onDropdownChanged(lv_event_t* event);
    static void onApplyButtonEvent(lv_event_t* event);
    static void onEditButtonEvent(lv_event_t* event);

    void updateSwatches(const core::UiTheme& selected);
    String computeSuggestedName(const core::UiTheme& theme) const;

    core::UiTheme theme_;
    std::function<void()> onCloseRequested_;
    std::function<std::vector<core::UiTheme>()> onThemeListRequested_;
    std::function<void(const core::UiTheme&)> onThemeApplyRequested_;
    std::function<void(const core::UiTheme&, const String&)> onThemeEditRequested_;

    std::vector<core::UiTheme> availableThemes_;
    lv_obj_t* schemeDropdown_ = nullptr;

    std::unique_ptr<DecoratedScreen> decorated_;

    static constexpr int kSwatchCount = 6;
    lv_obj_t* swatches_[kSwatchCount] = {};
};

} // namespace lightinator::ui::screens

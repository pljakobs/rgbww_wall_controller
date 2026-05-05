#pragma once

#include <lvgl.h>

#include <functional>
#include <memory>
#include <vector>

#include "ui/core/HsvColor.h"
#include "ui/core/UiTheme.h"
#include "ui/UiStateStore.h"
#include "ui/ScreenFactory.h"
#include "ui/screens/MainScreen.h"
#include "ui/screens/ColorPickerScreen.h"
#include "ui/screens/NetworkInfoScreen.h"
#include "ui/screens/ThemePreviewScreen.h"
#include "ui/screens/ThemeSelectorScreen.h"
#include "ui/screens/WifiConfigScreen.h"
#include "ui/screens/MenuTestScreen.h"

namespace lightinator::ui {

/**
 * AppNavigator — screen transition logic only.
 *
 * Owns all screen instance pointers. Drives showX / clearRoot.
 * Delegates screen construction to ScreenFactory.
 * Exposes raw accessors for visible-screen gating.
 */
class AppNavigator {
public:
    AppNavigator(lv_obj_t* root, UiStateStore& state, core::HsvColor& currentColor,
                 const core::UiTheme& theme,
                 std::function<bool(const core::UiTheme&)> onSaveTheme,
                 std::function<std::vector<core::UiTheme>()> onThemeList,
                 std::function<void(const core::UiTheme&)> onApplyTheme);

    void showMainScreen();
    void showColorPickerScreen();
    void showNetworkInfoScreen();
    void showWifiConfigScreen();
    void closeWifiConfigScreen();
    void showThemePreviewScreen();
        void showThemeSelectorScreen();
        void showThemeEditorScreen(const core::UiTheme& themeToEdit, const String& suggestedName);
    void showMenuTestScreen();
    void setTheme(const core::UiTheme& theme);

    screens::WifiConfigScreen*   wifiConfigScreen();
    screens::MainScreen*         mainScreen();
    screens::NetworkInfoScreen*  networkInfoScreen();

    /// Push current store state to whichever screen is active (called after navigation).
    void pushStateToActiveScreen();

    /// Register a callback that fires with the active NetworkInfoScreen pointer when
    /// the screen is shown, or nullptr when it is hidden (navigated away).
    void setOnNetworkInfoScreenChanged(
        std::function<void(screens::NetworkInfoScreen*)> cb);

private:
    enum class ActiveScreen {
        None,
        Main,
        ColorPicker,
        NetworkInfo,
        WifiConfig,
        ThemePreview,
        ThemeSelector,
        ThemeEditor,
        MenuTest,
    };

    void clearRoot();
    void remountActiveScreen();

    lv_obj_t*        root_;
    ScreenFactory    factory_;

    std::unique_ptr<screens::MainScreen>           mainScreen_;
    std::unique_ptr<screens::ColorPickerScreen>    colorPickerScreen_;
    std::unique_ptr<screens::WifiConfigScreen>     wifiConfigScreen_;
    std::unique_ptr<screens::NetworkInfoScreen>    networkInfoScreen_;
    std::unique_ptr<screens::ThemePreviewScreen>   themePreviewScreen_;
        std::unique_ptr<screens::ThemeSelectorScreen>  themeSelectorScreen_;
        /// themePreviewScreen_ is reused as the editor in the new two-screen flow.
    std::unique_ptr<screens::MenuTestScreen>   menuTestScreen_;

    std::function<void(screens::NetworkInfoScreen*)> onNetworkInfoScreenChanged_;
    ActiveScreen activeScreen_ = ActiveScreen::None;
};

} // namespace lightinator::ui

#pragma once

#include <functional>
#include <SmingCore.h>

#include "ui/AppUi.h"

namespace lightinator {

/**
 * UiRuntimeService — LVGL lock discipline and periodic tick.
 *
 * Centralises all lvgl_runtime_adapter_lock/unlock usage so callers
 * never touch the lock directly.  Owns the periodic tick timer that
 * drives LVGL animation.
 *
 * Use runOnUiThread() to execute any LVGL mutation from any context.
 */
class UiRuntimeService {
public:
    /// Execute fn() inside an LVGL lock/unlock pair.
    void runOnUiThread(std::function<void()> fn);

    /// Start the periodic tick timer (must be called after LVGL init).
    void start(ui::AppUi& ui);

    /// Stop the tick timer.
    void stop();

private:
    void tick();

    ui::AppUi*  ui_    = nullptr;
    SimpleTimer timer_;
};

} // namespace lightinator

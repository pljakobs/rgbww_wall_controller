#pragma once

#include <functional>
#include <SmingCore.h>

#include "ui/AppUi.h"

namespace lightinator {

/**
 * UiRuntimeService — LVGL lock discipline.
 *
 * Centralises all lvgl_runtime_adapter_lock/unlock usage so callers
 * never touch the lock directly.
 *
 * Use runOnUiThread() to execute any LVGL mutation from any context.
 */
class UiRuntimeService {
public:
    /// Execute fn() inside an LVGL lock/unlock pair.
    void runOnUiThread(std::function<void()> fn);

    /// Runtime lifecycle hook (reserved for future use).
    void start();

    /// Runtime lifecycle hook (reserved for future use).
    void stop();
};

} // namespace lightinator

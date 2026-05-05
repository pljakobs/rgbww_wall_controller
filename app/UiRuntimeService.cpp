#include "UiRuntimeService.h"

extern "C" {
#include "lvgl_runtime_adapter/lvgl_runtime_adapter.h"
}

namespace lightinator {

void UiRuntimeService::runOnUiThread(std::function<void()> fn)
{
    lvgl_runtime_adapter_lock();
    fn();
    lvgl_runtime_adapter_unlock();
}

void UiRuntimeService::start()
{
    // Intentionally empty: UI updates are fully event-driven.
}

void UiRuntimeService::stop()
{
    // Intentionally empty: retained for symmetric runtime lifecycle.
}

} // namespace lightinator

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

void UiRuntimeService::tick()
{
    if (ui_) {
        runOnUiThread([this]() { ui_->tickAnimation(); });
    }
}

void UiRuntimeService::start(ui::AppUi& ui)
{
    ui_ = &ui;
    timer_.initializeMs<180>([](void* self) {
        static_cast<UiRuntimeService*>(self)->tick();
    }, this).start();
}

void UiRuntimeService::stop()
{
    timer_.stop();
    ui_ = nullptr;
}

} // namespace lightinator

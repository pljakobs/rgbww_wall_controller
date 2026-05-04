#pragma once

#include "ui/AppUi.h"
#include "networking.h"
#include "ui/WifiConfigFlow.h"
#include "UiRuntimeService.h"

namespace lightinator {

// Encapsulates all AppWIFI -> AppUi callback registration.
// Keeps networking concerns out of the application entrypoint.
class NetworkUiBinder {
public:
    NetworkUiBinder(ui::AppUi& ui, AppWIFI& wifi, ui::WifiConfigFlow* wifiFlow,
                    UiRuntimeService& runtime)
        : ui_(ui), wifi_(wifi), wifiFlow_(wifiFlow), runtime_(runtime) {}

    void bind();
    void syncState();

private:
    void pushNeighboursOnUiThread();

    ui::AppUi&         ui_;
    AppWIFI&           wifi_;
    ui::WifiConfigFlow* wifiFlow_;
    UiRuntimeService&  runtime_;
    SimpleTimer        neighbourDebounce_;
};

} // namespace lightinator

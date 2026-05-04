#pragma once

#include "ui/AppUi.h"
#include "networking.h"
#include "ui/WifiConfigFlow.h"

namespace lightinator {

// Encapsulates all AppWIFI -> AppUi callback registration.
// Keeps networking concerns out of the application entrypoint.
class NetworkUiBinder {
public:
    NetworkUiBinder(ui::AppUi& ui, AppWIFI& wifi, ui::WifiConfigFlow* wifiFlow)
        : ui_(ui), wifi_(wifi), wifiFlow_(wifiFlow) {}

    void bind();
    void syncState();

private:
    ui::AppUi& ui_;
    AppWIFI& wifi_;
    ui::WifiConfigFlow* wifiFlow_;
};

} // namespace lightinator

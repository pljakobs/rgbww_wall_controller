#pragma once

#include <vector>
#include <WString.h>

#include "ui/screens/NetworkInfoScreen.h"

namespace lightinator::ui {

// Holds all canonical UI-facing state.
// No LVGL calls. Services write here; presenters read from here.
class UiStateStore {
public:
    bool wifiConnected() const { return wifiConnected_; }
    const String& ipAddress() const { return ipAddress_; }
    const String& netmask() const { return netmask_; }
    const String& gateway() const { return gateway_; }
    const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours() const { return neighbours_; }

    // Returns true if the value actually changed.
    bool setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway);
    bool setNeighbours(const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours);

private:
    bool wifiConnected_ = false;
    String ipAddress_;
    String netmask_;
    String gateway_;
    std::vector<screens::NetworkInfoScreen::Neighbour> neighbours_;
};

} // namespace lightinator::ui

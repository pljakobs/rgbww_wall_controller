#include "ui/UiStateStore.h"

namespace lightinator::ui {

bool UiStateStore::setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway)
{
    if (wifiConnected_ == connected &&
        ipAddress_ == ip &&
        netmask_ == netmask &&
        gateway_ == gateway) {
        return false;
    }
    wifiConnected_ = connected;
    ipAddress_ = ip;
    netmask_ = netmask;
    gateway_ = gateway;
    return true;
}

bool UiStateStore::setNeighbours(const std::vector<screens::NetworkInfoScreen::Neighbour>& neighbours)
{
    if (neighbours_.size() == neighbours.size()) {
        bool same = true;
        for (size_t i = 0; i < neighbours.size(); ++i) {
            if (neighbours_[i].name != neighbours[i].name ||
                neighbours_[i].ip != neighbours[i].ip) {
                same = false;
                break;
            }
        }
        if (same) {
            return false;
        }
    }
    neighbours_ = neighbours;
    return true;
}

} // namespace lightinator::ui

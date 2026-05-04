#include "NetworkUiBinder.h"

#include <esp_log.h>
#include <vector>

#include "ui/screens/NetworkInfoScreen.h"

static const char* BINDER_TAG = "NetUiBinder";

namespace lightinator {

static void push_network_state(ui::AppUi& ui, AppWIFI& wifi)
{
    ui.setNetworkInfo(wifi.isConnected(), wifi.ipAddress(), wifi.netmask(), wifi.gateway());

    std::vector<ui::screens::NetworkInfoScreen::Neighbour> neighbours;
    for (const auto& n : wifi.visibleNeighbours()) {
        neighbours.push_back({n.name, n.ip});
    }
    ui.setNeighbours(neighbours);
}

void NetworkUiBinder::bind()
{
    wifi_.setStatusCallback([this](const String& message) {
        ESP_LOGI(BINDER_TAG, "WiFi: %s", message.c_str());
        if (wifiFlow_) {
            wifiFlow_->onStatusMessage(message);
        }
    });

    wifi_.setScanCompletedCallback([this](const BssList& list) {
        if (wifiFlow_) {
            wifiFlow_->onScanCompleted(list);
        }
    });

    wifi_.setConnectedCallback([this]() {
        runtime_.runOnUiThread([this]() { push_network_state(ui_, wifi_); });
        if (wifiFlow_) {
            wifiFlow_->onWifiConnected();
        }
    });

    wifi_.setStationDisconnectedCallback([this]() {
        runtime_.runOnUiThread([this]() { push_network_state(ui_, wifi_); });
        if (wifiFlow_) {
            wifiFlow_->onWifiDisconnected();
        }
    });
}

void NetworkUiBinder::syncState()
{
    push_network_state(ui_, wifi_);
}

} // namespace lightinator

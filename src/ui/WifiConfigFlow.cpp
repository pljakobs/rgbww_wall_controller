#include "ui/WifiConfigFlow.h"

extern "C" {
#include "lvgl_runtime_adapter/lvgl_runtime_adapter.h"
}

#include "networking.h"
#include "ui/AppUi.h"
#include "ui/screens/WifiConfigScreen.h"

namespace lightinator::ui {

WifiConfigFlow::WifiConfigFlow(AppUi& ui, AppWIFI& wifi) : ui_(ui), wifi_(wifi)
{
}

void WifiConfigFlow::startIfNeeded()
{
    if(!shouldAutoOpen()) {
        return;
    }

    ensureScreenVisible(true);
}

void WifiConfigFlow::onWifiDisconnected()
{
    if(wifi_.isConnected()) {
        return;
    }

    ensureScreenVisible(true);

    lvgl_runtime_adapter_lock();
    if(auto* screen = ui_.wifiConfigScreen()) {
        screen->setStatusText(F("WiFi disconnected, please reconnect"));
    }
    lvgl_runtime_adapter_unlock();
}

bool WifiConfigFlow::shouldAutoOpen() const
{
    if(wifi_.isConnected()) {
        return false;
    }

    // If there is no saved STA config, always show onboarding.
    if(!wifi_.hasConfiguredStation()) {
        return true;
    }

    // For saved credentials, open onboarding only when connect fallback is active.
    return wifi_.isApActive() || wifi_.get_con_status() == CONNECTION_STATUS::ERROR;
}

void WifiConfigFlow::ensureScreenVisible(bool kickScan)
{
    lvgl_runtime_adapter_lock();
    if(ui_.wifiConfigScreen() == nullptr) {
        ui_.showWifiConfigScreen();
        bindScreenCallbacks();
    }

    if(auto* screen = ui_.wifiConfigScreen()) {
        if(kickScan) {
            screen->setStatusText(F("Scanning for WiFi networks..."));
        }

        const BssList cached = wifi_.getAvailableNetworks();
        if(cached.count() > 0) {
            screen->setNetworks(cached);
        }
    }
    lvgl_runtime_adapter_unlock();

    if(kickScan && !wifi_.isScanning()) {
        wifi_.scan(false);
    }
}

void WifiConfigFlow::onStatusMessage(const String& status)
{
    if(ui_.wifiConfigScreen() == nullptr && shouldAutoOpen()) {
        ensureScreenVisible(false);
    }

    lvgl_runtime_adapter_lock();
    if(auto* screen = ui_.wifiConfigScreen()) {
        screen->setStatusText(status);
    }
    lvgl_runtime_adapter_unlock();
}

void WifiConfigFlow::onScanCompleted(const BssList& networks)
{
    lvgl_runtime_adapter_lock();
    if(auto* screen = ui_.wifiConfigScreen()) {
        screen->setNetworks(networks);
        if(networks.count() > 0) {
            screen->setStatusText(F("Select a network and press connect"));
        } else {
            screen->setStatusText(F("No visible networks found, enter hidden SSID"));
        }
    }
    lvgl_runtime_adapter_unlock();
}

void WifiConfigFlow::onWifiConnected()
{
    lvgl_runtime_adapter_lock();
    if(ui_.wifiConfigScreen() != nullptr) {
        ui_.closeWifiConfigScreen();
    }
    lvgl_runtime_adapter_unlock();
}

void WifiConfigFlow::bindScreenCallbacks()
{
    auto* screen = ui_.wifiConfigScreen();
    if(screen == nullptr) {
        return;
    }

    screen->setOnConnectRequested([this](const String& ssid, const String& password) {
        wifi_.connect(ssid, password, true);
    });
}

} // namespace lightinator::ui

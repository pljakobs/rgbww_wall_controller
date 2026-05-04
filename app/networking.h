#pragma once

#include <SmingCore.h>
#include <app-config.h>
#include <functional>
#include <vector>

#include "controllers.h"
#include "mdnsHandler.h"

enum CONNECTION_STATUS {
    IDLE = 0,
    CONNECTING = 1,
    CONNECTED = 2,
    ERROR = 3,
};

class AppWIFI {
public:
    struct Neighbour {
        String name;
        String ip;
    };

    explicit AppWIFI(AppConfig& cfg);
    virtual ~AppWIFI() = default;

    void init();

    void connect(String ssid, String pass, bool new_con = false);
    void connect(String ssid, bool new_con = false);

    CONNECTION_STATUS get_con_status() const
    {
        return client_status_;
    }

    String get_con_err_msg() const
    {
        return client_err_msg_;
    }

    void startAp();
    void stopAp(int delay = 0);

    bool isApActive() const
    {
        return WifiAccessPoint.isEnabled();
    }

    void scan(bool connectAfterScan);

    bool isScanning() const
    {
        return scanning_;
    }

    BssList getAvailableNetworks() const;

    bool hasConfiguredStation() const
    {
        return WifiStation.getSSID().length() > 0;
    }

    bool isConnected() const
    {
        return WifiStation.isConnected();
    }

    String ipAddress() const
    {
        return WifiStation.getIP().toString();
    }

    String netmask() const
    {
        return WifiStation.getNetworkMask().toString();
    }

    String gateway() const
    {
        return WifiStation.getNetworkGateway().toString();
    }

    std::vector<Neighbour> visibleNeighbours() const;

    void forgetWifi();

    void setStatusCallback(std::function<void(const String&)> callback)
    {
        status_callback_ = std::move(callback);
    }

    void setScanCompletedCallback(std::function<void(const BssList&)> callback)
    {
        scan_completed_callback_ = std::move(callback);
    }

    void setConnectedCallback(std::function<void()> callback)
    {
        connected_callback_ = std::move(callback);
    }

    void setStationDisconnectedCallback(std::function<void()> callback)
    {
        station_disconnected_callback_ = std::move(callback);
    }

    /// Fired (on the main task) whenever the neighbour list changes due to mDNS activity.
    void setNeighboursChangedCallback(std::function<void()> callback)
    {
        controllers_.setOnChangedCallback(std::move(callback));
    }

    Controllers& controllers()
    {
        return controllers_;
    }

private:
    static String makeDefaultDeviceName();
    static String sanitizeName(const String& name);

    void STADisconnect(const String& ssid, MacAddress bssid, WifiDisconnectReason reason);
    void STAConnected(const String& ssid, MacAddress bssid, uint8_t channel);
    void STAGotIP(IpAddress ip, IpAddress mask, IpAddress gateway);
    void scanCompleted(bool succeeded, BssList& list);

    void broadcastWifiStatus();
    void broadcastWifiStatus(const String& message);

    AppConfig& cfg_;
    int con_ctr_ = 0;
    bool scanning_ = false;
    bool keep_sta_after_scan_ = false;
    bool new_connection_ = false;
    String client_err_msg_;
    Timer ap_timer_;
    Timer dns_start_timer_;
    BssList networks_;
    IpAddress ap_ip_;
    IpAddress ip_;
    DnsServer dns_server_;
    CONNECTION_STATUS client_status_ = CONNECTION_STATUS::IDLE;
    std::function<void(const String&)> status_callback_;
    std::function<void(const BssList&)> scan_completed_callback_;
    std::function<void()> connected_callback_;
    std::function<void()> station_disconnected_callback_;
    Controllers controllers_;
    mdnsHandler mdns_service_{controllers_};
};

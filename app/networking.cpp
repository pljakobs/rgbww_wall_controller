#include "networking.h"

#include <cctype>

namespace {
constexpr const char* kDefaultApIp = "192.168.4.1";
constexpr const char* kDefaultApSsidPrefix = "Lightinator_";
constexpr const char* kDefaultApPassword = "rgbwwctrl";
constexpr int kDefaultConnectionRetries = 10;
constexpr int kDnsPort = 53;

int compareBssByRssi(const BssInfo& a, const BssInfo& b)
{
    if(a.rssi > b.rssi) {
        return -1;
    }
    if(a.rssi < b.rssi) {
        return 1;
    }
    return 0;
}
} // namespace

AppWIFI::AppWIFI(AppConfig& cfg)
    : cfg_(cfg), ap_ip_(String(kDefaultApIp))
{
}

BssList AppWIFI::getAvailableNetworks() const
{
    return networks_;
}

std::vector<AppWIFI::Neighbour> AppWIFI::visibleNeighbours() const
{
    std::vector<Neighbour> neighbours;
    const uint32_t selfId = static_cast<uint32_t>(system_get_chip_id());

    for(const auto& entry : controllers_.entries()) {
        if(!entry.online || entry.id == selfId) {
            continue;
        }

        Neighbour neighbour;
        neighbour.name = entry.name;
        neighbour.ip = entry.ip;
        neighbours.push_back(std::move(neighbour));
    }

    return neighbours;
}

void AppWIFI::scan(bool connectAfterScan)
{
    scanning_ = true;
    keep_sta_after_scan_ = connectAfterScan;
    WifiStation.startScan(ScanCompletedDelegate(&AppWIFI::scanCompleted, this));
}

void AppWIFI::scanCompleted(bool succeeded, BssList& list)
{
    if(succeeded) {
        debug_i("AppWIFI::scanCompleted");
        networks_.clear();
        for(size_t i = 0; i < list.count(); i++) {
            if(!list[i].hidden && list[i].ssid.length() > 0) {
                networks_.add(list[i]);
            }
        }
        networks_.sort(compareBssByRssi);
    } else {
        debug_e("wifi scan failed");
    }

    scanning_ = false;
    if(scan_completed_callback_) {
        scan_completed_callback_(networks_);
    }

    if(keep_sta_after_scan_) {
        WifiStation.connect();
    }
}

void AppWIFI::forgetWifi()
{
    debug_i("AppWIFI::forgetWifi");
    WifiStation.config("", "");
    WifiStation.disconnect();

    if(!WifiAccessPoint.isEnabled()) {
        startAp();
    }

    client_status_ = CONNECTION_STATUS::IDLE;
}

void AppWIFI::init()
{
    debug_i("AppWIFI::init");

    if(!WifiStation.isEnabled()) {
        WifiStation.enable(true, true);
    }
    WifiStation.enable(true);

    if(WifiAccessPoint.isEnabled()) {
        WifiAccessPoint.enable(false, true);
    }

    {
        const String defaultName = makeDefaultDeviceName();
        AppConfig::Network network(cfg_);
        if(auto networkUpdate = network.update()) {
            if(network.ap.getSsid().length() == 0) {
                networkUpdate.ap.setSsid(defaultName);
            }
            if(network.ap.getPassword().length() == 0) {
                networkUpdate.ap.setPassword(kDefaultApPassword);
            }
            if(network.mdns.getName().length() == 0) {
                networkUpdate.mdns.setName(sanitizeName(defaultName));
            }
        }
        WifiAccessPoint.setIP(ap_ip_);
    }

    con_ctr_ = 0;

    WifiEvents.onStationDisconnect(StationDisconnectDelegate(&AppWIFI::STADisconnect, this));
    WifiEvents.onStationConnect(StationConnectDelegate(&AppWIFI::STAConnected, this));
    WifiEvents.onStationGotIP(StationGotIPDelegate(&AppWIFI::STAGotIP, this));

    if(WifiStation.getSSID().length() == 0) {
        debug_i("AppWIFI::init no stored station SSID, starting AP");
        startAp();
        scan(false);
        return;
    }

    {
        AppConfig::Network network(cfg_);
        if(!network.connection.getDhcp() && network.connection.getIp().length() != 0) {
            if(WifiStation.isEnabledDHCP()) {
                WifiStation.enableDHCP(false);
            }

            if(!(WifiStation.getIP() == network.connection.getIp()) ||
               !(WifiStation.getNetworkGateway() == network.connection.getGateway()) ||
               !(WifiStation.getNetworkMask() == network.connection.getNetmask())) {
                WifiStation.setIP(network.connection.getIp(), network.connection.getNetmask(), network.connection.getGateway());
            }
        } else if(!WifiStation.isEnabledDHCP()) {
            WifiStation.enableDHCP(true);
        }
    }

    WifiStation.connect();
    client_status_ = CONNECTION_STATUS::CONNECTING;
    broadcastWifiStatus(F("Connecting to WiFi"));
}

void AppWIFI::connect(String ssid, bool new_con)
{
    connect(std::move(ssid), "", new_con);
}

void AppWIFI::connect(String ssid, String pass, bool new_con)
{
    debug_i("AppWIFI::connect ssid=%s new=%d", ssid.c_str(), new_con);

    con_ctr_ = 0;
    new_connection_ = new_con;
    client_status_ = CONNECTION_STATUS::CONNECTING;

    WifiStation.config(ssid, pass);
    WifiStation.connect();
    broadcastWifiStatus(F("Connecting to WiFi"));
}

void AppWIFI::STADisconnect(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
    (void)ssid;
    (void)bssid;
    debug_i("AppWIFI::STADisconnect reason=%d retries=%d", static_cast<int>(reason), con_ctr_);

    if(con_ctr_ >= kDefaultConnectionRetries || WifiStation.getConnectionStatus() == eSCS_WrongPassword) {
        client_status_ = CONNECTION_STATUS::ERROR;
        client_err_msg_ = WifiStation.getConnectionStatusName();

        if(new_connection_) {
            WifiStation.disconnect();
            WifiStation.config("", "");
        } else {
            scan(true);
            startAp();
        }
    }

    broadcastWifiStatus(client_err_msg_);

    if(station_disconnected_callback_) {
        station_disconnected_callback_();
    }

    ++con_ctr_;
}

void AppWIFI::STAConnected(const String& ssid, MacAddress bssid, uint8_t channel)
{
    (void)bssid;
    (void)channel;

    debug_i("AppWIFI::STAConnected SSID=%s", ssid.c_str());

    String deviceName = makeDefaultDeviceName();
    {
        AppConfig::Network network(cfg_);
        if(network.ap.getSsid().length() > 0) {
            deviceName = network.ap.getSsid();
        }
    }

    WifiStation.setHostname(deviceName);

    {
        AppConfig::Network network(cfg_);
        if(auto networkUpdate = network.update()) {
            networkUpdate.mdns.setName(sanitizeName(deviceName));
        }
    }

    {
        AppConfig::Network network(cfg_);
        const String host = network.mdns.getName().length() > 0 ? network.mdns.getName() : sanitizeName(deviceName);
        mdns_service_.setHostname(host);
    }

    client_status_ = CONNECTION_STATUS::CONNECTED;
    con_ctr_ = 0;

    broadcastWifiStatus(F("Connected to WiFi"));
}

void AppWIFI::STAGotIP(IpAddress ip, IpAddress mask, IpAddress gateway)
{
    (void)mask;
    (void)gateway;

    ip_ = ip;

    mdns_service_.start();
    {
        AppConfig::Network network(cfg_);
        const uint32_t id = static_cast<uint32_t>(system_get_chip_id());
        const String host = network.mdns.getName().length() > 0 ? network.mdns.getName() : makeDefaultDeviceName();
        controllers_.addOrUpdate(id, host, ip.toString(), -1);
    }

    if(new_connection_) {
        stopAp(90000);
    } else {
        stopAp(1000);
    }

    if(connected_callback_) {
        connected_callback_();
    }

    broadcastWifiStatus();
}

void AppWIFI::stopAp(int delay)
{
    if(!WifiAccessPoint.isEnabled()) {
        return;
    }

    if(delay > 0) {
        ap_timer_.initializeMs(delay, std::bind(&AppWIFI::stopAp, this, 0)).startOnce();
        return;
    }

    ap_timer_.stop();
    dns_start_timer_.stop();

    if(WifiAccessPoint.isEnabled()) {
        WifiAccessPoint.enable(false, false);
    }

    broadcastWifiStatus(F("AP stopping"));
}

void AppWIFI::startAp()
{
    debug_i("AppWIFI::startAp");

    if(!WifiAccessPoint.isEnabled()) {
        WifiAccessPoint.enable(true, false);

        AppConfig::Network network(cfg_);
        const String ssid = network.ap.getSsid().length() ? network.ap.getSsid() : makeDefaultDeviceName();
        const String pass = network.ap.getPassword().length() ? network.ap.getPassword() : String(kDefaultApPassword);

        if(network.ap.getSecured()) {
            WifiAccessPoint.config(ssid, pass, AUTH_WPA2_PSK);
        } else {
            WifiAccessPoint.config(ssid, "", AUTH_OPEN);
        }
    }

    dns_start_timer_.initializeMs(500, [this]() {
        const IpAddress apIp = WifiAccessPoint.getIP();
        if(apIp.toString() != "0.0.0.0") {
            dns_server_.start(kDnsPort, "*", apIp);
            dns_start_timer_.stop();
        }
    }).start();

    broadcastWifiStatus(F("AP started"));
}

void AppWIFI::broadcastWifiStatus()
{
    broadcastWifiStatus(String());
}

void AppWIFI::broadcastWifiStatus(const String& message)
{
    if(status_callback_) {
        if(message.length() > 0) {
            status_callback_(message);
            return;
        }

        if(WifiStation.isConnected()) {
            status_callback_(F("WiFi connected: ") + WifiStation.getIP().toString());
            return;
        }

        if(WifiAccessPoint.isEnabled()) {
            status_callback_(F("AP active: ") + WifiAccessPoint.getIP().toString());
            return;
        }
    }
}

String AppWIFI::makeDefaultDeviceName()
{
    char buf[64];
    m_snprintf(buf, sizeof(buf), "%s%lu", kDefaultApSsidPrefix, static_cast<unsigned long>(system_get_chip_id()));
    return String(buf);
}

String AppWIFI::sanitizeName(const String& name)
{
    String out = name;
    for(unsigned i = 0; i < out.length(); ++i) {
        char c = out[i];
        if(c == ' ') {
            out[i] = '-';
            continue;
        }
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

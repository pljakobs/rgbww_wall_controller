#pragma once

#include <SmingCore.h>

class AppWIFI;

namespace lightinator::ui {

class AppUi;

class WifiConfigFlow {
public:
    WifiConfigFlow(AppUi& ui, AppWIFI& wifi);

    void startIfNeeded();
    void onWifiDisconnected();
    void onStatusMessage(const String& status);
    void onScanCompleted(const BssList& networks);
    void onWifiConnected();

private:
    void bindScreenCallbacks();
    bool shouldAutoOpen() const;
    void ensureScreenVisible(bool kickScan);

    AppUi& ui_;
    AppWIFI& wifi_;
};

} // namespace lightinator::ui

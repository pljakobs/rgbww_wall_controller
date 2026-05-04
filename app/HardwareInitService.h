#pragma once

namespace lightinator {

struct HardwareCapabilities {
    bool displayOk = false;
    bool touchOk   = false;
};

/**
 * HardwareInitService — backlight, display adapter, and touch init.
 *
 * Encapsulates all hardware bring-up for the Guition 4848S040 panel.
 * Returns a capabilities struct so the caller can decide how to proceed
 * without needing to know hardware-specific details.
 */
class HardwareInitService {
public:
    HardwareCapabilities init();
};

} // namespace lightinator

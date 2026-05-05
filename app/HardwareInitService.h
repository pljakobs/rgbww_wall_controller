#pragma once

#include <cstdint>

namespace lightinator {

struct HardwareTouchCalibration {
    bool enabled = false;
    float a = 1.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;
    float e = 1.0f;
    float f = 0.0f;
};

struct HardwareInitOptions {
    int brightnessPercent = 80;
    int backlightTimeoutSeconds = 30;
    uint16_t touchStablePressMs = 50;
    HardwareTouchCalibration touchCalibration = {};
};

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
    HardwareCapabilities init(const HardwareInitOptions& options = {});
    void setBacklightBrightness(int brightnessPercent);
    void setBacklightTimeoutSeconds(int timeoutSeconds);
    void applyTouchCalibration(const HardwareTouchCalibration& calibration);
    bool readTouch(int16_t* x, int16_t* y);

private:
    void applyBacklightOutput(bool enabled);

    int brightnessPercent_ = 80;
    int backlightTimeoutSeconds_ = 30;
    bool backlightAwake_ = true;
    bool swallowWakeTouch_ = false;
    int64_t lastTouchUs_ = 0;
};

} // namespace lightinator

#pragma once

#include <cstdint>

#include "HardwareInitService.h"
#include "ui/core/TouchCalibrationData.h"

class AppConfig;

namespace lightinator {

struct DisplaySettings {
    int brightness = 80;
    int timeout = 30;
    uint16_t touchStablePressMs = 50;
    HardwareTouchCalibration calibration = {};
};

DisplaySettings loadDisplaySettings(AppConfig* cfg);
void loadUiSettings(AppConfig* cfg, int& brightness, int& timeout);
bool saveDisplaySettings(AppConfig* cfg, HardwareInitService& hw, int brightness, int timeout);
void previewBacklightBrightness(HardwareInitService& hw, int brightness);
bool saveTouchCalibration(AppConfig* cfg,
                          HardwareInitService& hw,
                          const ui::core::TouchCalibrationCapture& capture);

} // namespace lightinator

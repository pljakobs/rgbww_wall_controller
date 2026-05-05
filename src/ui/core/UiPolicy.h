#pragma once

#include <cstdint>

namespace lightinator::ui::core::policy {

constexpr uint32_t kBrightnessPreviewIntervalMs = 20;

constexpr int kTimeoutSliderMinStep = 1;
constexpr int kTimeoutSliderMaxStep = 121;
constexpr int kTimeoutStepSeconds = 5;
constexpr int kTimeoutMaxSeconds = 600;

constexpr int kTouchCalibrationHitTolerancePx = 36;
constexpr int kTouchCalibrationTargetMarginPx = 40;

} // namespace lightinator::ui::core::policy

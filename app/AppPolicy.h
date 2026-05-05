#pragma once

#include <cstdint>

namespace lightinator::policy {

constexpr int kDefaultBrightnessPercent = 80;
constexpr int kDefaultBacklightTimeoutSeconds = 30;
constexpr uint16_t kDefaultTouchStablePressMs = 50;

constexpr int kMinBrightnessPercent = 0;
constexpr int kMaxBrightnessPercent = 100;

constexpr int kNeverBacklightTimeoutSeconds = -1;
constexpr int kMinBacklightTimeoutSeconds = 5;
constexpr int kMaxBacklightTimeoutSeconds = 600;

constexpr uint32_t kNetworkNeighboursDebounceMs = 300;

} // namespace lightinator::policy

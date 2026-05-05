#pragma once

#include <algorithm>
#include <cstdint>

namespace lightinator::ui::core {

constexpr uint32_t kHueScale = 10;
constexpr uint32_t kPercentScale = 10;
constexpr uint32_t kHueMax = 359 * kHueScale;
constexpr uint32_t kPercentMax = 100 * kPercentScale;

struct HsvColor {
    uint32_t h = 0;
    uint32_t s = kPercentMax;
    uint32_t v = kPercentMax;
};

inline HsvColor clampHsv(HsvColor color)
{
    color.h = std::min<uint32_t>(kHueMax, color.h);
    color.s = std::min<uint32_t>(kPercentMax, color.s);
    color.v = std::min<uint32_t>(kPercentMax, color.v);
    return color;
}

inline uint16_t toLvHue(uint32_t hue)
{
    return static_cast<uint16_t>(std::min<uint32_t>(359, (hue + (kHueScale / 2)) / kHueScale));
}

inline uint8_t toLvPercent(uint32_t percent)
{
    return static_cast<uint8_t>(std::min<uint32_t>(100, (percent + (kPercentScale / 2)) / kPercentScale));
}

inline uint32_t fromLvHue(uint16_t hue)
{
    return std::min<uint32_t>(kHueMax, static_cast<uint32_t>(hue) * kHueScale);
}

inline uint32_t fromLvPercent(uint8_t percent)
{
    return std::min<uint32_t>(kPercentMax, static_cast<uint32_t>(percent) * kPercentScale);
}

} // namespace lightinator::ui::core

#pragma once

#include <array>
#include <cstdint>

namespace lightinator::ui::core {

struct TouchCalibrationSample {
    int32_t referenceX = 0;
    int32_t referenceY = 0;
    int32_t rawX = 0;
    int32_t rawY = 0;
};

struct TouchCalibrationCapture {
    int32_t screenWidth = 0;
    int32_t screenHeight = 0;
    std::array<TouchCalibrationSample, 5> points = {};
};

struct TouchCalibrationMatrix {
    float a = 1.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;
    float e = 1.0f;
    float f = 0.0f;
};

} // namespace lightinator::ui::core
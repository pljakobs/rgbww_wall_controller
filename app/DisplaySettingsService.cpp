#include "DisplaySettingsService.h"

#include <array>

#include <app-config.h>

#include "AppPolicy.h"

namespace lightinator {

namespace {

using ui::core::TouchCalibrationCapture;
using ui::core::TouchCalibrationMatrix;

int clampInt(int value, int minValue, int maxValue)
{
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}

double absoluteDouble(double value)
{
    return (value < 0.0) ? -value : value;
}

float numberToFloat(ConfigDB::Number value)
{
    return static_cast<float>(ConfigDB::number_t::asFloat(value));
}

bool solveLinear3x3(double matrix[3][4], double solution[3])
{
    for (int pivot = 0; pivot < 3; ++pivot) {
        int bestRow = pivot;
        for (int row = pivot + 1; row < 3; ++row) {
            if (absoluteDouble(matrix[row][pivot]) > absoluteDouble(matrix[bestRow][pivot])) {
                bestRow = row;
            }
        }
        if (absoluteDouble(matrix[bestRow][pivot]) < 1e-9) {
            return false;
        }
        if (bestRow != pivot) {
            for (int col = pivot; col < 4; ++col) {
                const double temp = matrix[pivot][col];
                matrix[pivot][col] = matrix[bestRow][col];
                matrix[bestRow][col] = temp;
            }
        }

        const double divisor = matrix[pivot][pivot];
        for (int col = pivot; col < 4; ++col) {
            matrix[pivot][col] /= divisor;
        }
        for (int row = 0; row < 3; ++row) {
            if (row == pivot) {
                continue;
            }
            const double factor = matrix[row][pivot];
            for (int col = pivot; col < 4; ++col) {
                matrix[row][col] -= factor * matrix[pivot][col];
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        solution[i] = matrix[i][3];
    }
    return true;
}

bool solveTouchCalibration(const TouchCalibrationCapture& capture, TouchCalibrationMatrix& matrix)
{
    double sXX = 0.0;
    double sXY = 0.0;
    double sYY = 0.0;
    double sX = 0.0;
    double sY = 0.0;
    double rhsX0 = 0.0;
    double rhsX1 = 0.0;
    double rhsX2 = 0.0;
    double rhsY0 = 0.0;
    double rhsY1 = 0.0;
    double rhsY2 = 0.0;

    for (const auto& point : capture.points) {
        const double rawX = point.rawX;
        const double rawY = point.rawY;
        const double refX = point.referenceX;
        const double refY = point.referenceY;

        sXX += rawX * rawX;
        sXY += rawX * rawY;
        sYY += rawY * rawY;
        sX += rawX;
        sY += rawY;

        rhsX0 += refX * rawX;
        rhsX1 += refX * rawY;
        rhsX2 += refX;
        rhsY0 += refY * rawX;
        rhsY1 += refY * rawY;
        rhsY2 += refY;
    }

    double systemX[3][4] = {
        {sXX, sXY, sX, static_cast<double>(rhsX0)},
        {sXY, sYY, sY, static_cast<double>(rhsX1)},
        {sX, sY, static_cast<double>(capture.points.size()), static_cast<double>(rhsX2)},
    };
    double systemY[3][4] = {
        {sXX, sXY, sX, static_cast<double>(rhsY0)},
        {sXY, sYY, sY, static_cast<double>(rhsY1)},
        {sX, sY, static_cast<double>(capture.points.size()), static_cast<double>(rhsY2)},
    };

    double solutionX[3] = {};
    double solutionY[3] = {};
    if (!solveLinear3x3(systemX, solutionX) || !solveLinear3x3(systemY, solutionY)) {
        return false;
    }

    matrix.a = static_cast<float>(solutionX[0]);
    matrix.b = static_cast<float>(solutionX[1]);
    matrix.c = static_cast<float>(solutionX[2]);
    matrix.d = static_cast<float>(solutionY[0]);
    matrix.e = static_cast<float>(solutionY[1]);
    matrix.f = static_cast<float>(solutionY[2]);
    return true;
}

HardwareTouchCalibration toHardwareCalibration(const TouchCalibrationMatrix& matrix)
{
    HardwareTouchCalibration calibration = {};
    calibration.enabled = true;
    calibration.a = matrix.a;
    calibration.b = matrix.b;
    calibration.c = matrix.c;
    calibration.d = matrix.d;
    calibration.e = matrix.e;
    calibration.f = matrix.f;
    return calibration;
}

bool hasStoredCalibration(const AppConfig::Root& root)
{
    const auto& points = root.display.touchCalibration.points;
    return points.p1.raw.getX() != 0 || points.p1.raw.getY() != 0 ||
           points.p2.raw.getX() != 0 || points.p2.raw.getY() != 0 ||
           points.p3.raw.getX() != 0 || points.p3.raw.getY() != 0 ||
           points.p4.raw.getX() != 0 || points.p4.raw.getY() != 0 ||
           points.p5.raw.getX() != 0 || points.p5.raw.getY() != 0;
}

} // namespace

DisplaySettings loadDisplaySettings(AppConfig* cfg)
{
    DisplaySettings settings;
    if (cfg == nullptr) {
        return settings;
    }

    AppConfig::Root root(*cfg);
    settings.brightness = static_cast<int>(root.display.getBrightness());
    settings.timeout = static_cast<int>(root.display.getTimeout());
    settings.touchStablePressMs = static_cast<uint16_t>(root.display.getTouchStablePressMs());

    if (hasStoredCalibration(root)) {
        const auto& matrix = root.display.touchCalibration.matrix;
        settings.calibration.enabled = true;
        settings.calibration.a = numberToFloat(matrix.getA());
        settings.calibration.b = numberToFloat(matrix.getB());
        settings.calibration.c = numberToFloat(matrix.getC());
        settings.calibration.d = numberToFloat(matrix.getD());
        settings.calibration.e = numberToFloat(matrix.getE());
        settings.calibration.f = numberToFloat(matrix.getF());
    }

    return settings;
}

void loadUiSettings(AppConfig* cfg, int& brightness, int& timeout)
{
    const DisplaySettings settings = loadDisplaySettings(cfg);
    brightness = settings.brightness;
    timeout = settings.timeout;
}

bool saveDisplaySettings(AppConfig* cfg, HardwareInitService& hw, int brightness, int timeout)
{
    if (cfg == nullptr) {
        return false;
    }

    brightness = clampInt(brightness, policy::kMinBrightnessPercent, policy::kMaxBrightnessPercent);
    if (timeout != policy::kNeverBacklightTimeoutSeconds) {
        timeout = clampInt(timeout,
                           policy::kMinBacklightTimeoutSeconds,
                           policy::kMaxBacklightTimeoutSeconds);
    }

    AppConfig::Root root(*cfg);
    if (auto update = root.update()) {
        update.display.setBrightness(static_cast<uint8_t>(brightness));
        update.display.setTimeout(timeout);
    } else {
        return false;
    }

    hw.setBacklightBrightness(brightness);
    hw.setBacklightTimeoutSeconds(timeout);
    return true;
}

void previewBacklightBrightness(HardwareInitService& hw, int brightness)
{
    hw.setBacklightBrightness(brightness);
}

bool saveTouchCalibration(AppConfig* cfg,
                          HardwareInitService& hw,
                          const ui::core::TouchCalibrationCapture& capture)
{
    if (cfg == nullptr) {
        return false;
    }

    TouchCalibrationMatrix matrix = {};
    if (!solveTouchCalibration(capture, matrix)) {
        return false;
    }

    AppConfig::Root root(*cfg);
    if (auto update = root.update()) {
        update.display.touchCalibration.screen.setWidth(static_cast<uint32_t>(capture.screenWidth));
        update.display.touchCalibration.screen.setHeight(static_cast<uint32_t>(capture.screenHeight));
        update.display.touchCalibration.matrix.setA(ConfigDB::Number(matrix.a));
        update.display.touchCalibration.matrix.setB(ConfigDB::Number(matrix.b));
        update.display.touchCalibration.matrix.setC(ConfigDB::Number(matrix.c));
        update.display.touchCalibration.matrix.setD(ConfigDB::Number(matrix.d));
        update.display.touchCalibration.matrix.setE(ConfigDB::Number(matrix.e));
        update.display.touchCalibration.matrix.setF(ConfigDB::Number(matrix.f));

        auto storePoint = [](auto& pointUpdater, const ui::core::TouchCalibrationSample& point) {
            pointUpdater.reference.setX(static_cast<uint32_t>(point.referenceX));
            pointUpdater.reference.setY(static_cast<uint32_t>(point.referenceY));
            pointUpdater.raw.setX(static_cast<uint32_t>(point.rawX));
            pointUpdater.raw.setY(static_cast<uint32_t>(point.rawY));
        };

        storePoint(update.display.touchCalibration.points.p1, capture.points[0]);
        storePoint(update.display.touchCalibration.points.p2, capture.points[1]);
        storePoint(update.display.touchCalibration.points.p3, capture.points[2]);
        storePoint(update.display.touchCalibration.points.p4, capture.points[3]);
        storePoint(update.display.touchCalibration.points.p5, capture.points[4]);
    } else {
        return false;
    }

    hw.applyTouchCalibration(toHardwareCalibration(matrix));
    return true;
}

} // namespace lightinator

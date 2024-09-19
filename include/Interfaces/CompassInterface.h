#pragma once

class CompassInterface
{
public:
    virtual int GetAzimuth() = 0;

    virtual void PrintRawValues() = 0;

    // Calibration
    virtual void BeginCalibration() = 0;
    virtual void IterateCalibration() = 0;
    virtual void EndCalibration() = 0;
    virtual void GetCalibrationData(JsonDocument &doc) = 0;
    virtual void SetCalibrationData(JsonDocument &doc) = 0;
};
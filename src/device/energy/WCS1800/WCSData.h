// WCSData.h
#pragma once

#include <map>

namespace overseer::device::energy::data {
    struct WCSData {
        // Raw measurements
        float current = 0.0f;           // Current in Amps
        float voltage = 0.0f;           // Raw voltage reading
        float current_smooth = 0.0f;    // Smoothed current
        
        // Historical max tracking
        float max_current = 0.0f;       // Lifetime max current
        float max_current_dir = 0.0f;   // Direction of max current
        
        // Windowed max current tracking
        std::map<String, float> max_current_windows;
        
        // Sample statistics
        uint64_t total_samples = 0;
        uint64_t dropped_samples = 0;
        uint64_t bad_adc_read = 0;
        float samples_per_second = 0.0f;
        
        // Calibration data
        float zero_point_voltage = 0.0f;
        bool is_calibrated = false;
        
        // Sensor status
        bool valid_reading = true;
        unsigned long last_update_ms = 0;
    };
} // namespace overseer::device::energy::data




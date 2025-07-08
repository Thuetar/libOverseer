#pragma once
#ifndef MPU_DATA_H
#define MPU_DATA_H
#include <map>
#include <String>
#include <Arduino.h>

namespace mpu6000 {
    namespace data {

    struct GMaxWindow {
        float max_gx = 0.0f;
        float max_gy = 0.0f;
        float max_gz = 0.0f;
        
        // Max G-forces by Direction
        float dir_gx = 0.0f;  // signed value at max
        float dir_gy = 0.0f;
        float dir_gz = 0.0f;
        
        //Smoothed G's    
        float smooth_gx = 0.0f;
        float smooth_gy = 0.0f;
        float smooth_gz = 0.0f;
        
        unsigned long last_reset = 0;  // millis()
    };

    struct MPUData {
        // Current orientation and acceleration
        float pitch_deg = 0.0f;
        float roll_deg = 0.0f;
        float gx = 0.0f;
        float gy = 0.0f;
        float gz = 0.0f;

        // Smoothed versions
        float pitch_deg_smooth = 0.0f;
        float roll_deg_smooth = 0.0f;
        float gx_smooth = 0.0f;
        float gy_smooth = 0.0f;
        float gz_smooth = 0.0f;

        // For spike detection
        float gx_last = 0.0f;
        float gy_last = 0.0f;
        float gz_last = 0.0f;

        // Lifetime max G-forces
        float max_gx = 0.0f;
        float max_gy = 0.0f;
        float max_gz = 0.0f;
        
        // maps for historical G-force tracking:
        std::map<String, float> max_g_windows_x;
        std::map<String, float> max_g_windows_y;
        std::map<String, float> max_g_windows_z;
        
        // Rolling max windows (time durations in ms)
        GMaxWindow max_1s;
        GMaxWindow max_5s;
        GMaxWindow max_10s;
        GMaxWindow max_15s;
        GMaxWindow max_30s;
        GMaxWindow max_45s;
        GMaxWindow max_60s;
        GMaxWindow max_1m;
        GMaxWindow max_5m;
        GMaxWindow max_10m;
        GMaxWindow max_15m;
        GMaxWindow max_30m;
    };

    }  // namespace data
}  // namespace mpu6000

#endif

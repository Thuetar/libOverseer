//MPU6000.h
#pragma once

#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <math.h>
#include <Arduino.h>
#include <ArduinoLog.h>
#include "MPUData.h"
#include <deque>
#include <map>
#include <vector>
#include <algorithm>

using namespace overseer::device::imu::data;
namespace overseer::device::imu {
    class MPU6000 {
        private:
            bool initialized = false;
            MPU6050 mpu;
            MPUData _data;            
            void configureHardware();  // Wire, I2C, etc.
            std::deque<std::pair<unsigned long, float>> gx_history;
            std::deque<std::pair<unsigned long, float>> gy_history;
            std::deque<std::pair<unsigned long, float>> gz_history;
            //Comment, maybe... attempt to limit the size.            
            const std::vector<unsigned long> g_windows = {1, 5, 10, 15, 30, 45, 60, 300, 600, 900, 1800}; // Declare historical windows (in seconds).

            // Internal helpers to track max values
            void updateMax(float &max_val, float &dir_val, float new_val);            
            uint64_t last_sample_time_ms = 0;
            uint64_t total_samples = 0;
            uint64_t dropped_samples = 0;
            uint64_t bad_i2c_read = 0;

            float samples_per_second = 0.0f;

            // Configuration parameters for smoothing and spike rejection
            float smoothing_alpha = 0.1f;
            float spike_threshold = 0.3f;
        public:
            MPU6000(uint8_t sda_pin = 20, uint8_t scl_pin = 21);
            bool isInitialized();              //Getter
            bool begin();                      // Initialize hardware
            void update();                     // Update readings & max G windows
            void setData(const MPUData& newData);  // Inject external data (stub/testing)
            MPUData getData() const;    // Retrieve current sensor data            
            void smoothAndFilterMPUData(MPUData& data);
            void printMPUData(const MPUData& data);
            
    };

}  // namespace overseer::device::imu



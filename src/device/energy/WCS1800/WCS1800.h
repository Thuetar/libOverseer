// WCS1800.h
#pragma once
#include <config/ConfigManager.h>

#include "WCSData.h"
#include "ADS1X15.h"

#include <deque>
#include <map>
#include <vector>
#include <algorithm>

using namespace overseer::device::energy::data;
using namespace config;
namespace overseer::device::energy {
    class WCS1800 {
        private:
            bool initialized = false;
            WCSData _data;
            uint8_t analogPin;

            
            // Configuration parameters (loaded from config)
            float sensitivity;           // mV/A
            float zeroCurrentVoltage;    // Voltage at zero current
            float vccVoltage;            // Supply voltage
            uint16_t adcResolution;      // ADC resolution bits
            float calibrationOffset;     // Calibration offset
            float smoothing_alpha;       // Smoothing factor
            float spike_threshold;       // Spike rejection threshold
            
            // Historical data for windowed max calculations
            std::deque<std::pair<unsigned long, float>> current_history;
            const std::vector<unsigned long> current_windows = {1, 5, 10, 15, 30, 45, 60, 300, 600, 900, 1800}; // seconds
            
            // Sample tracking
            uint64_t last_sample_time_ms = 0;
            uint64_t total_samples = 0;
            uint64_t dropped_samples = 0;
            uint64_t bad_adc_read = 0;
            float samples_per_second = 0.0f;
            
            // Private helper methods
            void configureHardware();
            float voltageToAnalogValue(float voltage);
            float analogValueToVoltage(int analogValue);
            void updateMax(float &max_val, float &dir_val, float new_val);
            void loadConfiguration();
            
        public:
            WCS1800(uint8_t pin);
            WCS1800();
            bool isInitialized();
            bool begin();
            void update();
            void setData(const WCSData& newData);
            WCSData getData() const;
            void smoothAndFilterData(WCSData& data);
            void printWCSData(const WCSData& data);
            
            // Configuration methods
            void setCalibrationOffset(float offset);
            void setSensitivity(float sens);
            void calibrateZeroPoint(uint8_t samples = 100);
            
            // Direct reading methods (for manual use)
            float readRawVoltage();
            float readCurrent();
            float readCurrentSmoothed(uint8_t samples = 10);
            bool isValidReading(float current);
            
            // Getters
            float getSensitivity() const;
            float getZeroCurrentVoltage() const;
            uint8_t getPin() const;
    };
} // namespace overseer::device::energy




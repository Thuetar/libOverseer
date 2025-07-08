// WCS1800.cpp
#include "WCS1800.h"

using namespace overseer::device::energy::data;
using namespace config;


namespace overseer::device::energy {
    /*
    WCS1800::WCS1800()
        : initialized(false),
        analogPin(0),
        sensitivity(66.0f),
        zeroCurrentVoltage(2.5f),
        vccVoltage(5.0f),
        adcResolution(4096),
        calibrationOffset(0.0f),
        smoothing_alpha(0.1f),
        spike_threshold(1.5f)
    {
        zeroCurrentVoltage = vccVoltage / 2.0f;
    }
    */
    WCS1800::WCS1800() {
        analogPin = 0;
        sensitivity = 66.0f;          // WCS1800 default: 66mV/A
        zeroCurrentVoltage = 2.5f;    // V at 0A
        vccVoltage = 5.0f;
        adcResolution = 4096;
        calibrationOffset = 0.0f;
        smoothing_alpha = 0.1f;
        spike_threshold = 1.5f;
    }

    WCS1800::WCS1800(uint8_t pin) 
        : analogPin(pin)
        , sensitivity(66.0f)      // WCS1800 default: 66mV/A
        , vccVoltage(3.3f)
        , adcResolution(12)
        , calibrationOffset(0.0f)
        , smoothing_alpha(0.1f)
        , spike_threshold(0.3f)
    {
        zeroCurrentVoltage = vccVoltage / 2.0f;
    }

    bool WCS1800::begin() {
        Log.notice("WCS1800::INIT - Start on pin %d" CR, analogPin);
        
        configureHardware();
        loadConfiguration();
        
        // Test ADC reading
        int testRead = analogRead(analogPin);
        if (testRead < 0 || testRead > ((1 << adcResolution) - 1)) {
            Log.error("WCS1800: ADC test read failed" CR);
            return false;
        }
        
        Log.notice("WCS1800: Initialized successfully" CR);
        initialized = true;
        return true;
    }

    void WCS1800::configureHardware() {
        // Configure ADC resolution for ESP32
        analogReadResolution(adcResolution);
        
        // Set attenuation for 3.3V range on ESP32
        analogSetAttenuation(ADC_11db);
        
        // Set ADC width (ESP32 specific)
        //analogSetWidth(adcResolution);
        
        Log.trace("WCS1800: Hardware configured - %d-bit ADC, %.2fV range" CR, adcResolution, vccVoltage);
    }

    void WCS1800::loadConfiguration() {
        // Use default values - configuration can be set via setters
        // sensitivity = 66.0f; (already set in constructor)
        // vccVoltage = 3.3f; (already set in constructor)
        // calibrationOffset = 0.0f; (already set in constructor)
        // smoothing_alpha = 0.1f; (already set in constructor) 
        // spike_threshold = 0.3f; (already set in constructor)
        
        // Recalculate zero point based on loaded Vcc
        zeroCurrentVoltage = vccVoltage / 2.0f;
        
        Log.trace("WCS1800: Config loaded - Sens=%.1fmV/A, Vcc=%.2fV, Alpha=%.3f" CR,
                 sensitivity, vccVoltage, smoothing_alpha);
    }

    bool WCS1800::isInitialized() {
        return initialized;
    }

    void WCS1800::printWCSData(const WCSData& data) {
        Serial.println("=== WCS1800 CURRENT SENSOR DATA ===");
        Serial.printf("Current: %.3f A (Raw: %.3f V)\n", data.current, data.voltage);
        Serial.printf("Smooth Current: %.3f A\n", data.current_smooth);
        Serial.printf("Lifetime Max: %.3f A (dir: %.3f A)\n", data.max_current, data.max_current_dir);
        
        Serial.println("----- Calibration -----");
        Serial.printf("Zero Point: %.3f V, Calibrated: %s\n", 
                     data.zero_point_voltage, data.is_calibrated ? "Yes" : "No");
        
        Serial.println("----- Stats -----");
        Serial.printf("Samples: Total=%llu, Dropped=%llu, Bad ADC=%llu\n", 
                     data.total_samples, data.dropped_samples, data.bad_adc_read);
        Serial.printf("Sample Rate: %.2f Hz, Valid: %s\n", 
                     data.samples_per_second, data.valid_reading ? "Yes" : "No");
        
        Serial.println("----- Config -----");
        Serial.printf("Smoothing Alpha: %.3f, Spike Threshold: %.3f\n", 
                     smoothing_alpha, spike_threshold);
        
        Serial.println("--- Rolling Max Current Windows ---");
        for (const auto& entry : data.max_current_windows) {
            Serial.printf(" [%s] Max Current: %.3f A\n", entry.first.c_str(), entry.second);
        }
        Serial.println("====================================");
    }

    void WCS1800::smoothAndFilterData(WCSData& d) {
        // Exponential moving average
        d.current_smooth = smoothing_alpha * d.current + (1.0f - smoothing_alpha) * d.current_smooth;
        
        // Spike rejection
        if (abs(d.current_smooth - d.current) > spike_threshold) {
            d.current_smooth = d.current;
        }
        
        // Lifetime max tracking
        updateMax(d.max_current, d.max_current_dir, d.current_smooth);
        
        // Append to historical deque
        unsigned long now = millis();
        current_history.push_back({now, abs(d.current_smooth)});
        
        // Clear old samples outside the largest window
        auto cutoff = now - current_windows.back() * 1000;
        while (!current_history.empty() && current_history.front().first < cutoff) {
            current_history.pop_front();
        }
        
        // Compute per-window max
        d.max_current_windows.clear();
        for (auto window_sec : current_windows) {
            unsigned long window_start = now - window_sec * 1000;
            float max_current = 0.0f;
            
            for (const auto& entry : current_history) {
                if (entry.first >= window_start) {
                    max_current = std::max(max_current, entry.second);
                }
            }
            
            String label = String(window_sec) + "s";
            d.max_current_windows[label] = max_current;
        }
    }

    void WCS1800::update() {
        if (!initialized) return;
        
        unsigned long now = millis();
        total_samples++;
        
        // Read raw ADC value
        int rawValue = analogRead(analogPin);
        if (rawValue < 0) {
            bad_adc_read++;
            _data.valid_reading = false;
            return;
        }
        
        // Convert to voltage and current
        _data.voltage = analogValueToVoltage(rawValue);
        
        // Calculate current based on voltage difference from zero point
        float voltageDiff = _data.voltage - zeroCurrentVoltage;
        _data.current = (voltageDiff * 1000.0f) / sensitivity; // Convert mV to A
        _data.current += calibrationOffset; // Apply calibration
        
        // Validate reading
        _data.valid_reading = isValidReading(_data.current);
        
        // Update sample rate calculation
        if (last_sample_time_ms > 0) {
            unsigned long delta = now - last_sample_time_ms;
            if (delta > 0) {
                float current_rate = 1000.0f / delta;
                samples_per_second = (smoothing_alpha * current_rate) + 
                                   ((1.0f - smoothing_alpha) * samples_per_second);
            } else {
                dropped_samples++;
            }
        }
        last_sample_time_ms = now;
        
        // Store statistics in data structure
        _data.total_samples = total_samples;
        _data.dropped_samples = dropped_samples;
        _data.bad_adc_read = bad_adc_read;
        _data.samples_per_second = samples_per_second;
        _data.last_update_ms = now;
        _data.zero_point_voltage = zeroCurrentVoltage;
        
        // Apply smoothing and filtering
        smoothAndFilterData(_data);
    }

    float WCS1800::voltageToAnalogValue(float voltage) {
        return (voltage * ((1 << adcResolution) - 1)) / vccVoltage;
    }

    float WCS1800::analogValueToVoltage(int analogValue) {
        return (analogValue * vccVoltage) / ((1 << adcResolution) - 1);
    }

    float WCS1800::readRawVoltage() {
        int rawValue = analogRead(analogPin);
        return analogValueToVoltage(rawValue);
    }

    float WCS1800::readCurrent() {
        float voltage = readRawVoltage();
        float voltageDiff = voltage - zeroCurrentVoltage;
        float current = (voltageDiff * 1000.0f) / sensitivity;
        return current + calibrationOffset;
    }

    float WCS1800::readCurrentSmoothed(uint8_t samples) {
        float sum = 0.0f;
        for (uint8_t i = 0; i < samples; i++) {
            sum += readCurrent();
            delay(1);
        }
        return sum / samples;
    }

    bool WCS1800::isValidReading(float current) {
        // WCS1800 typical range: ±30A
        return (current >= -35.0f && current <= 35.0f);
    }

    void WCS1800::calibrateZeroPoint(uint8_t samples) {
        Log.notice("WCS1800: Calibrating zero point with %d samples..." CR, samples);
        float sum = 0.0f;
        
        for (uint8_t i = 0; i < samples; i++) {
            sum += readRawVoltage();
            delay(10);
        }
        
        zeroCurrentVoltage = sum / samples;
        _data.is_calibrated = true;
        _data.zero_point_voltage = zeroCurrentVoltage;
        
        // Configuration saving disabled - use external config management
        // configManager.setFloat("wcs1800", "zero_point_voltage", zeroCurrentVoltage);
        
        Log.notice("WCS1800: Zero point calibrated to %.3fV" CR, zeroCurrentVoltage);
    }

    void WCS1800::setCalibrationOffset(float offset) {
        calibrationOffset = offset;
        // Configuration saving disabled - use external config management
        // configManager.setFloat("wcs1800", "calibration_offset", offset);
    }

    void WCS1800::setSensitivity(float sens) {
        sensitivity = sens;
        // Configuration saving disabled - use external config management
        // configManager.setFloat("wcs1800", "sensitivity", sens);
    }

    void WCS1800::setData(const WCSData& newData) {
        _data = newData;
    }

    WCSData WCS1800::getData() const {
        return _data;
    }

    void WCS1800::updateMax(float& max_val, float& dir_val, float new_val) {
        float abs_val = fabs(new_val);
        if (abs_val > max_val) {
            max_val = abs_val;
            dir_val = new_val;
        }
    }

    // Getters
    float WCS1800::getSensitivity() const {
        return sensitivity;
    }

    float WCS1800::getZeroCurrentVoltage() const {
        return zeroCurrentVoltage;
    }

    uint8_t WCS1800::getPin() const {
        return analogPin;
    }

} // namespace overseer::device::energy





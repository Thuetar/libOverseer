// test/test_WCS1800.cpp
#include <unity.h>
//#include "arduino_compat.h"
#include "device/energy/WCS1800/WCS1800.h"
#include "config/ConfigManager.h"

using namespace overseer::device::energy;
using namespace overseer::device::energy::data;

// Mock filesystem for native testing
#ifndef ARDUINO
class MockFS {
public:
    // Minimal filesystem interface for testing
};
MockFS mockFS;
#else
#include <SPIFFS.h>
#endif

// Test fixtures and mocks
class MockConfigManager : public config::ConfigManager {
public:
#ifdef ARDUINO
    MockConfigManager() : config::ConfigManager(SPIFFS, "/test_config.ini") {}
#else
    MockConfigManager() : config::ConfigManager((fs::FS&)mockFS, "/test_config.ini") {}
#endif
    
    // Override methods for testing
    float getFloat(const char* section, const char* key, float defaultValue) const override {
        if (strcmp(section, "wcs1800") == 0) {
            if (strcmp(key, "sensitivity") == 0) return 66.0f;
            if (strcmp(key, "vcc_voltage") == 0) return 3.3f;
            if (strcmp(key, "calibration_offset") == 0) return 0.0f;
            if (strcmp(key, "smoothing_alpha") == 0) return 0.1f;
            if (strcmp(key, "spike_threshold") == 0) return 0.3f;
        }
        return defaultValue;
    }
    
    bool setFloat(const char* section, const char* key, float value) override {
        // Mock implementation - just return true
        return true;
    }
};

// Global test objects
WCS1800* testSensor = nullptr;
MockConfigManager* mockConfig = nullptr;

// Mock ADC readings for testing
static int mockAdcValue = 2048; // Mid-scale for 12-bit ADC
static bool mockAdcFail = false;

// Override Arduino functions for testing
#ifndef ARDUINO
int analogRead(uint8_t pin) {
    (void)pin;
    if (mockAdcFail) return -1;
    return mockAdcValue;
}
#endif

void setUp(void) {
    // Setup before each test
    mockConfig = new MockConfigManager();
    testSensor = new WCS1800(34); // Use GPIO 34
    mockAdcValue = 2048; // Reset to mid-scale
    mockAdcFail = false;
}

void tearDown(void) {
    // Cleanup after each test
    delete testSensor;
    delete mockConfig;
    testSensor = nullptr;
    mockConfig = nullptr;
}

// ============================================================================
// CONSTRUCTOR TESTS
// ============================================================================

void test_constructor_default_values(void) {
    WCS1800 sensor(25);
    
    TEST_ASSERT_EQUAL(25, sensor.getPin());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 66.0f, sensor.getSensitivity());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.65f, sensor.getZeroCurrentVoltage()); // 3.3V/2
    TEST_ASSERT_FALSE(sensor.isInitialized());
}

void test_constructor_custom_pin(void) {
    WCS1800 sensor(39);
    TEST_ASSERT_EQUAL(39, sensor.getPin());
}

// ============================================================================
// INITIALIZATION TESTS
// ============================================================================

void test_begin_successful_initialization(void) {
    bool result = testSensor->begin();
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(testSensor->isInitialized());
}

void test_begin_adc_failure(void) {
    mockAdcFail = true;
    bool result = testSensor->begin();
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(testSensor->isInitialized());
}

// ============================================================================
// VOLTAGE CONVERSION TESTS  
// ============================================================================

void test_voltage_conversion_mid_scale(void) {
    testSensor->begin();
    mockAdcValue = 2048; // Mid-scale for 12-bit (4096/2)
    
    float voltage = testSensor->readRawVoltage();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.65f, voltage); // Should be ~Vcc/2
}

void test_voltage_conversion_full_scale(void) {
    testSensor->begin();
    mockAdcValue = 4095; // Full scale for 12-bit
    
    float voltage = testSensor->readRawVoltage();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.3f, voltage); // Should be ~Vcc
}

void test_voltage_conversion_zero_scale(void) {
    testSensor->begin();
    mockAdcValue = 0;
    
    float voltage = testSensor->readRawVoltage();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, voltage);
}

// ============================================================================
// CURRENT MEASUREMENT TESTS
// ============================================================================

void test_current_reading_zero_current(void) {
    testSensor->begin();
    mockAdcValue = 2048; // Mid-scale = zero current
    
    float current = testSensor->readCurrent();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, current);
}

void test_current_reading_positive_current(void) {
    testSensor->begin();
    // Simulate 1A positive current: Vout = Vcc/2 + (Current * Sensitivity)
    // Vout = 1.65V + (1A * 0.066V/A) = 1.716V
    // ADC = 1.716V * 4095 / 3.3V ≈ 2129
    mockAdcValue = 2129;
    
    float current = testSensor->readCurrent();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, current);
}

void test_current_reading_negative_current(void) {
    testSensor->begin();
    // Simulate -1A current: Vout = 1.65V - 0.066V = 1.584V
    // ADC = 1.584V * 4095 / 3.3V ≈ 1967
    mockAdcValue = 1967;
    
    float current = testSensor->readCurrent();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, -1.0f, current);
}

void test_current_reading_with_calibration_offset(void) {
    testSensor->begin();
    testSensor->setCalibrationOffset(0.5f); // Add 0.5A offset
    mockAdcValue = 2048; // Zero current reading
    
    float current = testSensor->readCurrent();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, current);
}

// ============================================================================
// SMOOTHED READING TESTS
// ============================================================================

void test_smoothed_current_reading(void) {
    testSensor->begin();
    mockAdcValue = 2048; // Zero current
    
    float smoothed = testSensor->readCurrentSmoothed(5);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, smoothed);
}

// ============================================================================
// VALIDATION TESTS
// ============================================================================

void test_valid_reading_within_range(void) {
    testSensor->begin();
    
    TEST_ASSERT_TRUE(testSensor->isValidReading(0.0f));
    TEST_ASSERT_TRUE(testSensor->isValidReading(15.0f));
    TEST_ASSERT_TRUE(testSensor->isValidReading(-15.0f));
    TEST_ASSERT_TRUE(testSensor->isValidReading(30.0f));
    TEST_ASSERT_TRUE(testSensor->isValidReading(-30.0f));
}

void test_invalid_reading_out_of_range(void) {
    testSensor->begin();
    
    TEST_ASSERT_FALSE(testSensor->isValidReading(40.0f));
    TEST_ASSERT_FALSE(testSensor->isValidReading(-40.0f));
    TEST_ASSERT_FALSE(testSensor->isValidReading(100.0f));
}

// ============================================================================
// CALIBRATION TESTS
// ============================================================================

void test_zero_point_calibration(void) {
    testSensor->begin();
    mockAdcValue = 2100; // Slightly off from mid-scale
    
    testSensor->calibrateZeroPoint(10);
    
    // Zero point should be updated
    float expectedVoltage = (2100 * 3.3f) / 4095.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.01f, expectedVoltage, testSensor->getZeroCurrentVoltage());
}

void test_sensitivity_configuration(void) {
    testSensor->begin();
    testSensor->setSensitivity(100.0f); // Custom sensitivity
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, testSensor->getSensitivity());
}

// ============================================================================
// DATA STRUCTURE TESTS
// ============================================================================

void test_data_structure_initialization(void) {
    testSensor->begin();
    WCSData data = testSensor->getData();
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.current);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.voltage);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.current_smooth);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.max_current);
    TEST_ASSERT_EQUAL_UINT64(0, data.total_samples);
    TEST_ASSERT_FALSE(data.is_calibrated);
    TEST_ASSERT_TRUE(data.valid_reading);
}

void test_data_injection(void) {
    testSensor->begin();
    
    WCSData testData;
    testData.current = 5.5f;
    testData.voltage = 2.0f;
    testData.current_smooth = 5.3f;
    testData.total_samples = 100;
    
    testSensor->setData(testData);
    WCSData retrievedData = testSensor->getData();
    
    TEST_ASSERT_EQUAL_FLOAT(5.5f, retrievedData.current);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, retrievedData.voltage);
    TEST_ASSERT_EQUAL_FLOAT(5.3f, retrievedData.current_smooth);
    TEST_ASSERT_EQUAL_UINT64(100, retrievedData.total_samples);
}

// ============================================================================
// UPDATE MECHANISM TESTS
// ============================================================================

void test_update_increments_sample_count(void) {
    testSensor->begin();
    
    WCSData dataBefore = testSensor->getData();
    uint64_t samplesBefore = dataBefore.total_samples;
    
    testSensor->update();
    
    WCSData dataAfter = testSensor->getData();
    TEST_ASSERT_EQUAL_UINT64(samplesBefore + 1, dataAfter.total_samples);
}

void test_update_handles_adc_failure(void) {
    testSensor->begin();
    mockAdcFail = true;
    
    testSensor->update();
    
    WCSData data = testSensor->getData();
    TEST_ASSERT_FALSE(data.valid_reading);
    TEST_ASSERT_GREATER_THAN_UINT64(0, data.bad_adc_read);
}

void test_update_calculates_current_correctly(void) {
    testSensor->begin();
    mockAdcValue = 2129; // Should give ~1A
    
    testSensor->update();
    
    WCSData data = testSensor->getData();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, data.current);
    TEST_ASSERT_TRUE(data.valid_reading);
}

// ============================================================================
// SMOOTHING AND FILTERING TESTS
// ============================================================================

void test_smoothing_algorithm(void) {
    testSensor->begin();
    
    WCSData data;
    data.current = 5.0f;
    data.current_smooth = 0.0f;
    
    testSensor->smoothAndFilterData(data);
    
    // Should be smoothed towards the new value
    TEST_ASSERT_GREATER_THAN_FLOAT(0.0f, data.current_smooth);
    TEST_ASSERT_LESS_THAN_FLOAT(5.0f, data.current_smooth);
}

void test_spike_rejection(void) {
    testSensor->begin();
    
    WCSData data;
    data.current = 100.0f; // Large spike
    data.current_smooth = 1.0f; // Previous smooth value
    
    testSensor->smoothAndFilterData(data);
    
    // Spike should be rejected, smooth value should jump to current
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, data.current_smooth);
}

void test_max_current_tracking(void) {
    testSensor->begin();
    
    WCSData data;
    data.current = 10.0f;
    data.current_smooth = 10.0f;
    data.max_current = 5.0f;
    
    testSensor->smoothAndFilterData(data);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, data.max_current);
}

// ============================================================================
// WINDOWED MAXIMUM TESTS
// ============================================================================

void test_windowed_maximum_calculation(void) {
    testSensor->begin();
    
    WCSData data;
    data.current = 8.0f;
    data.current_smooth = 8.0f;
    
    testSensor->smoothAndFilterData(data);
    
    // Check that windowed maximums are populated
    TEST_ASSERT_FALSE(data.max_current_windows.empty());
    TEST_ASSERT_TRUE(data.max_current_windows.count("1s") > 0);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 8.0f, data.max_current_windows["1s"]);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

void test_full_sensor_lifecycle(void) {
    // Test complete sensor operation cycle
    TEST_ASSERT_TRUE(testSensor->begin());
    TEST_ASSERT_TRUE(testSensor->isInitialized());
    
    // Simulate some readings
    mockAdcValue = 2048; // Zero current
    testSensor->update();
    
    mockAdcValue = 2129; // ~1A
    testSensor->update();
    
    WCSData data = testSensor->getData();
    TEST_ASSERT_EQUAL_UINT64(2, data.total_samples);
    TEST_ASSERT_TRUE(data.valid_reading);
    TEST_ASSERT_GREATER_THAN_FLOAT(0.0f, data.samples_per_second);
}

void test_multiple_updates_sample_rate(void) {
    testSensor->begin();
    
    // Perform multiple updates
    for (int i = 0; i < 10; i++) {
        testSensor->update();
        delay(10); // Small delay to simulate real timing
    }
    
    WCSData data = testSensor->getData();
    TEST_ASSERT_EQUAL_UINT64(10, data.total_samples);
    TEST_ASSERT_GREATER_THAN_FLOAT(0.0f, data.samples_per_second);
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

void test_uninitialized_sensor_update(void) {
    // Test update without initialization
    WCSData dataBefore = testSensor->getData();
    testSensor->update(); // Should not crash
    WCSData dataAfter = testSensor->getData();
    
    // Should remain unchanged
    TEST_ASSERT_EQUAL_UINT64(dataBefore.total_samples, dataAfter.total_samples);
}

void test_invalid_pin_handling(void) {
    WCS1800 invalidSensor(255); // Invalid pin
    TEST_ASSERT_EQUAL(255, invalidSensor.getPin());
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_update_performance(void) {
    testSensor->begin();
    
    unsigned long startTime = micros();
    for (int i = 0; i < 100; i++) {
        testSensor->update();
    }
    unsigned long endTime = micros();
    
    unsigned long totalTime = endTime - startTime;
    unsigned long avgTime = totalTime / 100;
    
    // Update should complete in reasonable time (< 1ms per call)
    TEST_ASSERT_LESS_THAN_UINT32(1000, avgTime);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

void runAllTests(void) {
    UNITY_BEGIN();
    
    // Constructor tests
    RUN_TEST(test_constructor_default_values);
    RUN_TEST(test_constructor_custom_pin);
    
    // Initialization tests
    RUN_TEST(test_begin_successful_initialization);
    RUN_TEST(test_begin_adc_failure);
    
    // Voltage conversion tests
    RUN_TEST(test_voltage_conversion_mid_scale);
    RUN_TEST(test_voltage_conversion_full_scale);
    RUN_TEST(test_voltage_conversion_zero_scale);
    
    // Current measurement tests
    RUN_TEST(test_current_reading_zero_current);
    RUN_TEST(test_current_reading_positive_current);
    RUN_TEST(test_current_reading_negative_current);
    RUN_TEST(test_current_reading_with_calibration_offset);
    
    // Smoothed reading tests
    RUN_TEST(test_smoothed_current_reading);
    
    // Validation tests
    RUN_TEST(test_valid_reading_within_range);
    RUN_TEST(test_invalid_reading_out_of_range);
    
    // Calibration tests
    RUN_TEST(test_zero_point_calibration);
    RUN_TEST(test_sensitivity_configuration);
    
    // Data structure tests
    RUN_TEST(test_data_structure_initialization);
    RUN_TEST(test_data_injection);
    
    // Update mechanism tests
    RUN_TEST(test_update_increments_sample_count);
    RUN_TEST(test_update_handles_adc_failure);
    RUN_TEST(test_update_calculates_current_correctly);
    
    // Smoothing and filtering tests
    RUN_TEST(test_smoothing_algorithm);
    RUN_TEST(test_spike_rejection);
    RUN_TEST(test_max_current_tracking);
    
    // Windowed maximum tests
    RUN_TEST(test_windowed_maximum_calculation);
    
    // Integration tests
    RUN_TEST(test_full_sensor_lifecycle);
    RUN_TEST(test_multiple_updates_sample_rate);
    
    // Error handling tests
    RUN_TEST(test_uninitialized_sensor_update);
    RUN_TEST(test_invalid_pin_handling);
    
    // Performance tests
    RUN_TEST(test_update_performance);
    
    UNITY_END();
}

// ============================================================================
// MAIN FUNCTION FOR NATIVE TESTS
// ============================================================================

#ifndef ARDUINO
int main() {
    runAllTests();
    return 0;
}
#endif

// ============================================================================
// PLATFORMIO TEST CONFIGURATION
// ============================================================================

/*
; platformio.ini test configuration
[env:native]
platform = native
test_framework = unity
test_filter = test_WCS1800
build_flags = 
    -DUNITY_INCLUDE_CONFIG_H
    -DUNIT_TEST
lib_deps = 
    throwtheswitch/Unity@^2.5.2

[env:esp32_test]
platform = espressif32
board = esp32dev
framework = arduino
test_framework = unity
test_filter = test_WCS1800
lib_deps = 
    throwtheswitch/Unity@^2.5.2
    adafruit/ArduinoLog@^1.1.1
upload_port = /dev/ttyUSB0
monitor_speed = 115200
*/
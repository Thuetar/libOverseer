//DHFAMILY.h
#pragma once
#include <deque>
#include <map>
#include <vector>
#include "device/BaseSensorDevice.h"
//#include "../BaseSensorDevice.h"  // The base class we designed
#include "DHTDATA.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

using namespace overseer::device;
using namespace overseer::device::environment;
using namespace overseer::device::environment::data;
namespace overseer::device::environment {
    class DHTFAMILY : public BaseSensorDevice<DHTDATA> {
    private:
        DHT _dht;
        uint8_t _pin;
        uint8_t _dhttype;
        
        // Historical data for windowed analysis
        std::deque<std::pair<unsigned long, float>> humidity_history;
        std::deque<std::pair<unsigned long, float>> temperature_history;
        
        // DHT-specific configuration
        unsigned long read_interval_ms = 2000;  // DHT sensors need 2s between reads
        unsigned long last_read_attempt = 0;
        
    protected:
        void configureHardware() override {
            _dht.begin();
            Log.infoln("DHT sensor configured on pin %d, type %d", _pin, _dhttype);
        }
        
        bool readSensorData() override {
            unsigned long current_time = millis();
            
            // DHT sensors require minimum 2s between readings
            if (current_time - last_read_attempt < read_interval_ms) {
                return false;  // Too soon to read again
            }
            
            last_read_attempt = current_time;
            
            float h = _dht.readHumidity();
            float t = _dht.readTemperature();
            
            // Check if reads are valid
            if (isnan(h) || isnan(t)) {
                bad_reads++;
                return false;
            }
            
            // Update data structure
            _data.humidity = h;
            _data.temperature = t;
            _data.heat_index = _dht.computeHeatIndex(t, h, false);
            _data.valid_reading = true;
            _data.last_update_ms = current_time;
            _data.sample_time_ms = current_time;
            
            return true;
        }
        
        void processSensorData() override {
            unsigned long current_time = millis();
            
            // Add to historical data for windowed analysis
            humidity_history.push_back({current_time, _data.humidity});
            temperature_history.push_back({current_time, _data.temperature});
            
            // Update windowed max values
            updateWindowedMax(humidity_history, _data.max_humidity_windows, current_time);
            updateWindowedMax(temperature_history, _data.max_temperature_windows, current_time);
            
            // Update lifetime max values
            updateMax(_data.max_humidity, _data.max_humidity, _data.humidity);
            updateMax(_data.max_temperature, _data.max_temperature, _data.temperature);
            updateMax(_data.max_heat_index, _data.max_heat_index, _data.heat_index);
            
            // Copy sample tracking from base class
            _data.total_samples = total_samples;
            _data.dropped_samples = dropped_samples;
            _data.bad_reads = bad_reads;
            _data.samples_per_second = samples_per_second;
        }
        
    public:
        DHTFAMILY() : _dht(2, DHT11), _pin(2), _dhttype(DHT11) {}
        DHTFAMILY(uint8_t pin, uint8_t dhttype) : _dht(pin, dhttype), _pin(pin), _dhttype(dhttype) {}
        DHTFAMILY(uint8_t pin) : _dht(pin, DHT11), _pin(pin), _dhttype(DHT11) {}
        
        void smoothAndFilterData(DHTDATA& data) override {
            // Apply smoothing to humidity and temperature
            applySmoothingFilter(data.humidity, data.humidity_smooth);
            applySmoothingFilter(data.temperature, data.temperature_smooth);
            
            // DHT sensors are generally stable, light spike rejection
            applySpikeRejection(data.humidity, data.humidity_smooth, data.humidity);
            applySpikeRejection(data.temperature, data.temperature_smooth, data.temperature);
        }
        
        void printSensorData(const DHTDATA& data) override {
            Log.verboseln("DHT Data:");
            Log.verboseln("  Temperature: %.2f°C (smooth: %.2f°C)", data.temperature, data.temperature_smooth);
            Log.verboseln("  Humidity: %.2f%% (smooth: %.2f%%)", data.humidity, data.humidity_smooth);
            Log.verboseln("  Heat Index: %.2f°C", data.heat_index);
            Log.verboseln("  Samples: %lu total, %lu dropped, %lu bad reads", 
                         (unsigned long)data.total_samples, 
                         (unsigned long)data.dropped_samples, 
                         (unsigned long)data.bad_reads);
            Log.verboseln("  Sample Rate: %.2f Hz", data.samples_per_second);
            Log.verboseln("  Max Values: T=%.2f°C, H=%.2f%%, HI=%.2f°C", 
                         data.max_temperature, data.max_humidity, data.max_heat_index);
        }
        
        // DHT-specific methods
        uint8_t getPin() const { return _pin; }
        uint8_t getDHTType() const { return _dhttype; }
        void setReadInterval(unsigned long interval_ms) { read_interval_ms = interval_ms; }
        unsigned long getReadInterval() { return read_interval_ms; }
        unsigned long getLastReadAttempt() { return last_read_attempt; }
    };
}

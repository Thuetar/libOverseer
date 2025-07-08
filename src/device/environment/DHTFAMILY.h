//DHFAMILY.h
#pragma once
#include <deque>
#include <map>
#include <vector>

//#include <Arduino.h>
//#include <ArduinoLog.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
//#include "DHTFAMILY.h"

#ifdef ARDUINO

//#include "DHT.h"
#endif

namespace overseer::device::environment {
    struct DHTDATA
    {
        // Raw measurements
        float humidity = 0.0f;           // Current in Amps
        float temperature = 0.0f;           // Raw voltage reading
        uint64_t sample_time_ms = 0;
    };
    
    class DHTFAMILY {
        private:
            bool initialized = false;
            DHT _dht;
            uint8_t _pin;
            uint8_t _dhttype;            
            //void configureHardware();  // Wire, I2C, etc.
            //std::deque<std::pair<unsigned long, float>> gx_history;
            
            // Internal helpers to track max values
            //void updateMax(float &max_val, float &dir_val, float new_val);            
            uint64_t last_sample_time_ms = 0;
            uint64_t total_samples = 0;
            uint64_t dropped_samples = 0;
            uint64_t bad_reads = 0;
            float samples_per_second = 0.0f;
            
            // Configuration parameters for smoothing and spike rejection
            float smoothing_alpha = 0.1f;
            float spike_threshold = 0.3f;
        public:
            DHTFAMILY();
            DHTFAMILY(uint8_t, uint8_t);    
            DHTFAMILY(uint8_t);
            bool isInitialized();               //Getter
            int getTotalSamples();             //Getter
            bool begin();                       // Initialize hardware
            void update();                      // Update readings & max G windows
            const DHTDATA getData() const;            // Retrieve current sensor data            
            //void setData(const MPUData& newData);  // Inject external data (stub/testing)
            
            //void smoothAndFilterMPUData(MPUData& data);
            //void printMPUData(const MPUData& data);
            
    };

}  // namespace overseer::device::environment



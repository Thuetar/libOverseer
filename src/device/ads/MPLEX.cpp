// MPLEX.cpp
#include "esp_mac.h"
#include "MPLEX.h"

namespace overseer::device::ads {

    MPLEX::MPLEX() : _ads(0x48) {
        // Any additional setup if needed
    }
    bool MPLEX::begin() {    
        _ads.begin();
        _ads.setWireClock(100000);
        delay(30);
        _ads.setGain(0);
        _ads.setDataRate(7);  //  0 = slow   4 = medium   7 = fast
        //_ads.setMode(1);
        return isConnected();
    }

    bool MPLEX::isConnected() {
        return _ads.isConnected();        
    }

    void MPLEX::scan_i2c_bus () {
        Serial.println("I2C Scanner:: Starting");
        for (byte address = 1; address < 127; address++) {
            Wire.beginTransmission(address);
            if (Wire.endTransmission() == 0) {
                Serial.print("Found device at 0x");
                Serial.println(address, HEX);
            }
            delay(10);            
        }
        Serial.println("I2C Scanner:: Finished");
    }

    int16_t MPLEX::get_raw_reading( int8_t channel ) {
        return( _ads.readADC(channel));  // Read from channel 
    }
    ENERGY_READING MPLEX::get_energy_reading (int8_t channel) {
        ENERGY_READING energy_reading;
        energy_reading.current = get_current(channel);
        energy_reading.voltage = get_voltage(channel);
        return energy_reading;
    }
    
    float MPLEX::get_raw_voltage ( int8_t channel ) {
        //return( _ads.toVoltage(_ads.readADC(channel)) );
        int16_t raw = _ads.readADC(channel);  // Read from channel 0        
        float voltage_mV = _ads.toVoltage(raw);
        return voltage_mV;
    }

    float MPLEX::calibrate_zero(uint8_t samples) {
        float sum = 0.0f;
        for (uint8_t i = 0; i < samples; ++i) {
            int16_t raw = _ads.readADC(0);
            float voltage_mV = _ads.toVoltage(raw);
            sum += voltage_mV / 1000.0f;
            delay(10); // allow small settling time between reads
        }
        _zero_voltage = sum / samples;
        return (_zero_voltage);
    }

    float MPLEX::get_voltage( int8_t channel ) {
        int16_t raw = _ads.readADC(channel);  // Read from channel 0        
        float voltage_mV = _ads.toVoltage(raw);
        float voltage_V = voltage_mV / 1000.0f;
        return voltage_V;
    }

    float MPLEX::get_current( int8_t channel ) {
        //float amps = (get_voltage(channel) - 2.5f) / 0.066f;  // Adjust for sensor type
        //return amps;
        float current_A = (get_voltage(channel) - _zero_voltage) / 0.066f;
        return current_A;
    }


} // namespace overseer::device::ads
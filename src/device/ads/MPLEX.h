// MPLEX.h
#pragma once
#include <ADS1X15.h>

namespace overseer::device::ads {
    struct ENERGY_READING
    {
        /* data */
        float voltage = 0.00;
        float current = 0.00;
    } ; // emergy_reading
    
    class MPLEX {
    private:
        //ADS1115 _ads = ADS(0x48);
        //ADS1115 _ads(0x48);
        ADS1115 _ads; //(0x48);
        int8_t _voltage_factor = 1;
        float _zero_voltage = 2.5f; // default fallback baseline

    public:
        MPLEX();
        bool begin();
        bool isConnected();
        float get_zero_voltage ();
        int16_t get_raw_reading (int8_t);
        float get_raw_voltage (int8_t);
        float get_voltage (int8_t);
        float get_current (int8_t);
        ENERGY_READING get_energy_reading (int8_t);
        float calibrate_zero(uint8_t samples = 10);
        void scan_i2c_bus ();
    };
} // namespace overseer::device::ads




//system_config.h
#pragma once

#include "devices/IMU/MPU6000/MPU6000.h"    
#include "devices/IMU/MPU6000/MPU6000_instance.h"


struct RUNNING_CONFIG
{
    bool debug_enable = true;
    struct DEBUG_OPTIONS {
        bool enable_imu_print = true;
        unsigned long imu_log_message_interval = 5000;
        unsigned long imu_log_last_print_time = 0;
    };

    struct HARDWARE_CONFIG
    {
        struct SYSTEM_DISPLAY
        {
            bool headless = false;
            bool enable = true;
            struct DISPLAY_CONFIG
            {
                //uint16_t const model = ST7796S;
                uint8_t const TFT_MISO = 50;
                uint8_t const TFT_MOSI = 51;
                uint8_t const TFT_SCLK = 52;

                uint8_t const TFT_CS = A5;
                uint8_t const TFT_DC = A3;
                uint8_t const TFT_RST = A4;

                uint8_t const TFT_LED = -1;                            
            };            
            DISPLAY_CONFIG display_config;
        };        
        SYSTEM_DISPLAY system_display;
        
        // TODO: complete. struct OPTIONAL_DEVICE_CONFIG { };

        struct IMU
        {
            bool enable_imu = true;
            unsigned long update_interval = 500;
            unsigned long last_update_time = 0;
            mpu6000::MPU6000* mpu = nullptr;   
            struct PINS {
                uint8_t sda = 20; 
                uint8_t scl = 21;            
            };
            struct FILTER_CONFIG {
                float smoothing_alpha = 0.1f;           // EMA alpha
                float spike_threshold = 3.0f;           // G spike rejection threshold
                float window_smoothing_alpha = 0.5f;    // smoothing factor for G max windows
            };

            PINS pins;
            FILTER_CONFIG filter_config;
        };
        IMU imu;
    };

    DEBUG_OPTIONS debug_options;    
    HARDWARE_CONFIG hardware_config;
}; 
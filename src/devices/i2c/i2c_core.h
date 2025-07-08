#pragma once

#include <Arduino.h>
#include <Wire.h>

class I2CCore {
    private:
        bool _isInitialized = false;
    public:
        I2CCore() = default;

        void address_scan(){
            Serial.println("I2C Scanner");
            for (byte address = 1; address < 127; address++) {
                Wire.beginTransmission(address);
                if (Wire.endTransmission() == 0) {
                Serial.print("Found device at 0x");
                Serial.println(address, HEX);
                }
                delay(10);
            }
        }
            
        /**
         * Initialize I2C bus with given frequency (default 100kHz)
         * Optionally specify SDA and SCL pins (if platform supports it)
         */
        bool begin(uint32_t freq = 100000, uint8_t sdaPin = SDA, uint8_t sclPin = SCL) {
            #if defined(ESP32) || defined(ESP8266)
                Wire.begin(sdaPin, sclPin);
            #else
                Wire.begin();
            #endif
            Wire.setClock(freq);
            this->_isInitialized = true;
            return true;
        }
    
};

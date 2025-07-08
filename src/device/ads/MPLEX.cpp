// MPLEX.cpp
#include "esp_mac.h"
#include "MPLEX.h"

namespace overseer::device::ads {

    MPLEX::MPLEX(uint8_t address) : _ads(address) {
        _channelConfigs.resize(4);
        _channelData.resize(4);
        
        // Initialize default channel configurations
        for (int i = 0; i < 4; i++) {
            _channelConfigs[i].label = "CH" + String(i);
            _channelConfigs[i].units = "V";
        }
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

    int16_t MPLEX::getChannelRaw(int channel) {
        if (!isValidChannel(channel)) return 0;
        updateChannel(channel);
        return _channelData[channel].raw_value;
    }

    float MPLEX::getChannelVoltage(int channel) {
        if (!isValidChannel(channel)) return 0.0f;
        updateChannel(channel);
        return _channelData[channel].voltage;
    }

    float MPLEX::getChannelScaled(int channel) {
        if (!isValidChannel(channel)) return 0.0f;
        updateChannel(channel);
        return _channelData[channel].scaled_value;
    }

    bool MPLEX::isChannelValid(int channel) {
        if (!isValidChannel(channel)) return false;
        return _channelData[channel].valid;
    }

    void MPLEX::setChannelConfig(int channel, const ChannelConfig& config) {
        if (isValidChannel(channel)) {
            _channelConfigs[channel] = config;
        }
    }

    ChannelConfig MPLEX::getChannelConfig(int channel) const {
        if (isValidChannel(channel)) {
            return _channelConfigs[channel];
        }
        return ChannelConfig{};
    }

    void MPLEX::setChannelLabel(int channel, const String& label) {
        if (isValidChannel(channel)) {
            _channelConfigs[channel].label = label;
        }
    }

    void MPLEX::setChannelUnits(int channel, const String& units) {
        if (isValidChannel(channel)) {
            _channelConfigs[channel].units = units;
        }
    }

    void MPLEX::setChannelGain(int channel, float gain) {
        if (isValidChannel(channel)) {
            _channelConfigs[channel].gain = gain;
        }
    }

    void MPLEX::setChannelOffset(int channel, float offset) {
        if (isValidChannel(channel)) {
            _channelConfigs[channel].offset = offset;
        }
    }

    void MPLEX::setChannelRange(int channel, float min_val, float max_val) {
        if (isValidChannel(channel)) {
            _channelConfigs[channel].min_range = min_val;
            _channelConfigs[channel].max_range = max_val;
        }
    }

    void MPLEX::enableChannel(int channel, bool enabled) {
        if (isValidChannel(channel)) {
            _channelConfigs[channel].enabled = enabled;
        }
    }

    float MPLEX::calibrateChannelZero(int channel, uint8_t samples) {
        if (!isValidChannel(channel)) return 0.0f;
        
        float sum = 0.0f;
        for (uint8_t i = 0; i < samples; ++i) {
            int16_t raw = _ads.readADC(channel);
            float voltage_mV = _ads.toVoltage(raw);
            sum += voltage_mV / 1000.0f;
            delay(10);
        }
        float zero_voltage = sum / samples;
        _channelConfigs[channel].offset = zero_voltage;
        return zero_voltage;
    }

    void MPLEX::calibrateAllChannels(uint8_t samples) {
        for (int i = 0; i < 4; i++) {
            if (_channelConfigs[i].enabled) {
                calibrateChannelZero(i, samples);
            }
        }
    }

    ChannelData MPLEX::getChannelData(int channel) {
        if (!isValidChannel(channel)) {
            return ChannelData{};
        }
        updateChannel(channel);
        return _channelData[channel];
    }

    void MPLEX::updateAllChannels() {
        for (int i = 0; i < 4; i++) {
            updateChannel(i);
        }
    }

    void MPLEX::updateChannel(int channel) {
        if (!isValidChannel(channel) || !_channelConfigs[channel].enabled) {
            _channelData[channel].valid = false;
            return;
        }

        _channelData[channel].raw_value = _ads.readADC(channel);
        float voltage_mV = _ads.toVoltage(_channelData[channel].raw_value);
        _channelData[channel].voltage = voltage_mV / 1000.0f;
        
        // Apply gain and offset
        _channelData[channel].scaled_value = (_channelData[channel].voltage + _channelConfigs[channel].offset) * _channelConfigs[channel].gain;
        
        _channelData[channel].valid = true;
        _channelData[channel].last_update = millis();
    }

    void MPLEX::setGain(uint8_t gain) {
        _gain = gain;
        _ads.setGain(gain);
    }

    void MPLEX::setDataRate(uint8_t rate) {
        _dataRate = rate;
        _ads.setDataRate(rate);
    }

    void MPLEX::setWireClock(uint32_t clock) {
        _wireClock = clock;
        _ads.setWireClock(clock);
    }

    bool MPLEX::isValidChannel(int channel) const {
        return (channel >= 0 && channel < 4);
    }


} // namespace overseer::device::ads
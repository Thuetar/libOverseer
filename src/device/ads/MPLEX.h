// MPLEX.h
#pragma once
#include <ADS1X15.h>
#include <vector>

namespace overseer::device::ads {
    struct ChannelConfig {
        bool enabled = true;
        float gain = 1.0f;
        float offset = 0.0f;
        String label = "";
        String units = "V";
        float min_range = 0.0f;
        float max_range = 5.0f;
    };

    struct ChannelData {
        int16_t raw_value = 0;
        float voltage = 0.0f;
        float scaled_value = 0.0f;
        bool valid = false;
        unsigned long last_update = 0;
    };
    
    class MPLEX {
    private:
        ADS1115 _ads;
        std::vector<ChannelConfig> _channelConfigs;
        std::vector<ChannelData> _channelData;
        
        // ADC settings
        uint8_t _gain = 0;
        uint8_t _dataRate = 7;
        uint32_t _wireClock = 100000;

    public:
        MPLEX(uint8_t address = 0x48);
        
        // Initialization and status
        bool begin();
        bool isConnected();
        void scan_i2c_bus();
        
        // Generic ADC operations
        int16_t getChannelRaw(int channel);
        float getChannelVoltage(int channel);
        float getChannelScaled(int channel);
        bool isChannelValid(int channel);
        
        // Channel configuration
        void setChannelConfig(int channel, const ChannelConfig& config);
        ChannelConfig getChannelConfig(int channel) const;
        void setChannelLabel(int channel, const String& label);
        void setChannelUnits(int channel, const String& units);
        void setChannelGain(int channel, float gain);
        void setChannelOffset(int channel, float offset);
        void setChannelRange(int channel, float min_val, float max_val);
        void enableChannel(int channel, bool enabled = true);
        
        // Calibration
        float calibrateChannelZero(int channel, uint8_t samples = 10);
        void calibrateAllChannels(uint8_t samples = 10);
        
        // Data access
        ChannelData getChannelData(int channel);
        void updateAllChannels();
        void updateChannel(int channel);
        
        // ADC settings
        void setGain(uint8_t gain);
        void setDataRate(uint8_t rate);
        void setWireClock(uint32_t clock);
        uint8_t getGain() const { return _gain; }
        uint8_t getDataRate() const { return _dataRate; }
        uint32_t getWireClock() const { return _wireClock; }
        
        // Utility
        bool isValidChannel(int channel) const;
        int getChannelCount() const { return 4; }
    };
} // namespace overseer::device::ads




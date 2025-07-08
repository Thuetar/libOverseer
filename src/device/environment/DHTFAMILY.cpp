//DHFAMILY.cpp
#include "DHTFAMILY.h"

using namespace overseer::device::environment;
namespace overseer::device::environment {
    DHTFAMILY::DHTFAMILY() : _dht(2, DHT11) {  }
    DHTFAMILY::DHTFAMILY(uint8_t pin, uint8_t dhttype) : _dht(pin, dhttype) {  }
    DHTFAMILY::DHTFAMILY(uint8_t pin) : _dht(pin, DHT11) {  }

    bool DHTFAMILY::begin() {
        //Log.notice("WCS1800::INIT - Start on pin %d" CR, analogPin);
        if (isInitialized() == true) return true;
            //Log.verbose("WCS1800::INIT - Start on pin %d returned True CR", analogPin);


    }

    void DHTFAMILY::update() {

    }

    const DHTDATA DHTFAMILY::getData() const {
        DHTDATA data;
        data.humidity = 1.23;
        data.temperature = 2.345;
        return data;
    }
    bool DHTFAMILY::isInitialized() {
        return initialized;
    }

    int DHTFAMILY::getTotalSamples() {
        return total_samples;
    }
}


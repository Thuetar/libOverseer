#ifndef ARDUINO_COMPAT_H
#define ARDUINO_COMPAT_H

#ifndef ARDUINO
// Minimal Arduino compatibility for native testing
#include <stdint.h>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <string>

// Arduino String compatibility
class String {
public:
    String() : data_("") {}
    String(const char* str) : data_(str) {}
    String(const std::string& str) : data_(str) {}
    String(int val) : data_(std::to_string(val)) {}
    String(unsigned long val) : data_(std::to_string(val)) {}
    String(float val) : data_(std::to_string(val)) {}
    String(float val, int decimals) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.*f", decimals, val);
        data_ = buffer;
    }
    
    const char* c_str() const { return data_.c_str(); }
    size_t length() const { return data_.length(); }
    
    String operator+(const String& other) const {
        return String(data_ + other.data_);
    }
    
    bool operator==(const String& other) const {
        return data_ == other.data_;
    }
    
    bool operator<(const String& other) const {
        return data_ < other.data_;
    }
    
private:
    std::string data_;
};

// Basic types
typedef uint8_t byte;
typedef bool boolean;

// ESP32 ADC types (for native testing)
typedef enum {
    ADC_ATTEN_DB_0   = 0,
    ADC_ATTEN_DB_2_5 = 1,
    ADC_ATTEN_DB_6   = 2,
    ADC_ATTEN_DB_11  = 3,
} adc_attenuation_t;

// ESP32 ADC constants
#define ADC_11db ADC_ATTEN_DB_11

// Common Arduino constants
#define CR "\r\n"

// Pin states
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Common Arduino functions (stubs for testing)
inline void pinMode(uint8_t pin, uint8_t mode) { (void)pin; (void)mode; }
inline void digitalWrite(uint8_t pin, uint8_t val) { (void)pin; (void)val; }
inline int digitalRead(uint8_t pin) { (void)pin; return LOW; }
int analogRead(uint8_t pin); // Implemented in test files
inline void analogWrite(uint8_t pin, int val) { (void)pin; (void)val; }
inline void analogReadResolution(uint8_t bits) { (void)bits; }
inline void analogSetAttenuation(adc_attenuation_t attenuation) { (void)attenuation; }
inline void analogSetWidth(uint8_t bits) { (void)bits; }
inline unsigned long millis() { return 0; }
inline unsigned long micros() { return 0; }
inline void delay(unsigned long ms) { (void)ms; }
inline void delayMicroseconds(unsigned int us) { (void)us; }

// Serial stub
class SerialClass {
public:
    void begin(long baud) { (void)baud; }
    void print(const char* str) { printf("%s", str); }
    void println(const char* str) { printf("%s\n", str); }
    void print(int val) { printf("%d", val); }
    void println(int val) { printf("%d\n", val); }
    void print(float val) { printf("%.2f", val); }
    void println(float val) { printf("%.2f\n", val); }
    
    // Printf method for more complex formatting
    template<typename... Args>
    void printf(const char* format, Args... args) { 
        ::printf(format, args...); 
    }
};

extern SerialClass Serial;

#endif // ARDUINO

#endif // ARDUINO_COMPAT_H
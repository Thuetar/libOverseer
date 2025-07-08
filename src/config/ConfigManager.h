//config/ConfigManager.h
#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#include <FS.h>
#include <ArduinoLog.h>
#else
#include "../test/arduino_compat.h"
// Mock filesystem namespace for native testing
class File {
public:
    operator bool() const { return false; }
    size_t size() const { return 0; }
    size_t readBytes(char* buffer, size_t length) { (void)buffer; (void)length; return 0; }
    size_t print(const char* str) { (void)str; return 0; }
    void close() {}
};

namespace fs {
    class FS {
    public:
        File open(const char* path, const char* mode) { 
            (void)path; (void)mode; 
            return File(); 
        }
    };
}

// Mock ArduinoLog for native testing
class LogClass {
public:
    void notice(const char* format) { ::printf("%s", format); }
    template<typename... Args>
    void notice(const char* format, Args... args) { ::printf(format, args...); }
    
    void warning(const char* format) { ::printf("WARNING: %s", format); }
    template<typename... Args>
    void warning(const char* format, Args... args) { ::printf("WARNING: "); ::printf(format, args...); }
    
    void error(const char* format) { ::printf("ERROR: %s", format); }
    template<typename... Args>
    void error(const char* format, Args... args) { ::printf("ERROR: "); ::printf(format, args...); }
    
    void trace(const char* format) { ::printf("TRACE: %s", format); }
    template<typename... Args>
    void trace(const char* format, Args... args) { ::printf("TRACE: "); ::printf(format, args...); }
    
    void info(const char* format) { ::printf("INFO: %s", format); }
    template<typename... Args>
    void info(const char* format, Args... args) { ::printf("INFO: "); ::printf(format, args...); }
};
extern LogClass Log;

#define LOG_LEVEL_VERBOSE 5
#endif

#include <SimpleIni.h>

namespace config {
    class ConfigManager {
    public:
        ConfigManager(fs::FS &fs, const String &filePath = "/config.ini");

        bool begin();
        bool load();
        bool save();

        void set(const String &section, const String &key, const String &value);
        String get(const String &section, const String &key, const String &defaultValue = "") const;
        
        virtual const char* getString(const char* section, const char* key, const char* defaultValue = "") const;
        virtual int getInt(const char* section, const char* key, int defaultValue = 0) const;
        virtual bool getBool(const char* section, const char* key, bool defaultValue = false) const;
        virtual float getFloat(const char* section, const char* key, float defaultValue) const;
        virtual bool setFloat(const char* section, const char* key, float value);
        virtual bool setBool(const char* section, const char* key, bool value);

        void setVersion(const String &version);
        String getVersion() const;
        int8_t getLogLevel() const;

    private:
        fs::FS &_fs;
        String _filePath;
        mutable CSimpleIniA _ini;
        bool _dirty = false;
        String _version = "1.0.0";
        int8_t _logLevel;
        bool writeToDisk();
    };
}
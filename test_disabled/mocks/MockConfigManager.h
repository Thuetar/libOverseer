#ifndef MOCK_CONFIG_MANAGER_H
#define MOCK_CONFIG_MANAGER_H

#include <stdint.h>

// Mock version of ConfigManager for testing
class ConfigManager {
public:
    bool loadConfig();
    bool saveConfig();
    void setParameter(const char* key, const char* value);
    const char* getParameter(const char* key);
    
private:
    // Mock data storage
    static char mockData[256];
};

#endif
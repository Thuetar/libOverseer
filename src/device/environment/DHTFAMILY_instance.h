//DHTFAMILY_instance.h
#pragma once
#include "device/environment/DHTFAMILY.h"

namespace overseer::device::environment {

// Singleton-style instance accessor — not a member function!
inline DHTFAMILY& getInstance() {
    static DHTFAMILY instance;
    return instance;
}

} // namespace 

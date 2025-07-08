// WCS1800_instance.h
#pragma once
#include "WCS1800.h"

namespace overseer::device::energy {

// Singleton-style instance accessor — not a member function!
inline WCS1800& getInstance() {
    //static WCS1800 instance(34); // Default GPIO pin 34
    static WCS1800 instance;
    return instance;
}

} // namespace overseer::device::current

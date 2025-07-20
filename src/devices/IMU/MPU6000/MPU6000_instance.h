//MPU6000_instance.h
#pragma once
#include "MPU6000.h"

namespace overseer::device::imu {

// Singleton-style instance accessor — not a member function!
inline MPU6000& getInstance() {
    static MPU6000 instance;
    return instance;
}

} // namespace 

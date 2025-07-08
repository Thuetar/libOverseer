#include "MPU6000_stub.h"
#include <math.h>

namespace mpu6000 {
namespace stub {

    // simulated state
    float pitch_deg = 0.0f;
    float roll_deg = 0.0f;
    float gx = 0.0f;
    float gy = 0.0f;
    float gz = -9.81f;

    namespace {
        float angle = 0.0f;
    }

    void init() {
        // No-op in stub
    }

    void update() {
        angle += 0.04f;
        pitch_deg = 15.0f * sin(angle);
        roll_deg = 25.0f * cos(angle * 0.8f);

        gx = 0.1f * sin(angle * 2);
        gy = 0.1f * cos(angle * 1.5f);
        gz = -9.81f + 0.05f * sin(angle * 1.1f);
    }

    } // namespace stub
} // namespace mpu6000

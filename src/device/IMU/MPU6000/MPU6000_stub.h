#pragma once

namespace mpu6000 {
namespace stub {

extern float pitch_deg;
extern float roll_deg;
extern float gx;
extern float gy;
extern float gz;

void init();    // no-op for stub
void update();  // simulate values for testing
}  // namespace stub
}  // namespace mpu6000

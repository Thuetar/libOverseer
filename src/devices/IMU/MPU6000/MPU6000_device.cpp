#include "MPU6000.h"

namespace mpu6000 {

    MPU6000::MPU6000(uint8_t sda_pin, uint8_t scl_pin) {
        // You can add custom Wire pins initialization here if needed.
    }

    bool MPU6000::init() {
        Serial.println("MPU6000::INIT - Start");
        Wire.begin();
        delay(300);
        mpu.initialize();
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);  // or _4, _8, _16
        mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);

        #if MPU_RUN_DEVICE_TEST_CONNECT 
            unsigned long start = millis();
            while (!mpu.testConnection()) {
                if (millis() - start > 5000) {
                    Serial.println("MPU not responding...");
                    return false;
                }
                delay(300);
            }
        #else
            Serial.println("Skipping MPU connection test.");
        #endif

        Log.setLevel (5);

        initialized = true;
        return true;
    }

    bool MPU6000::isInititalized() {
        return initialized;
    }

    void MPU6000::printMPUData(const data::MPUData& data) {
        Serial.println("=== IMU DATA REPORT ===");
        Serial.printf("Pitch: %.2f deg, Roll: %.2f deg\n", data.pitch_deg, data.roll_deg);
        Serial.printf("Raw G:    gx=%.3f, gy=%.3f, gz=%.3f\n", data.gx, data.gy, data.gz);
        Serial.printf("Smooth G: gx=%.3f, gy=%.3f, gz=%.3f\n", data.gx_smooth, data.gy_smooth, data.gz_smooth);

        Serial.printf("Lifetime Max G: gx=%.3f, gy=%.3f, gz=%.3f\n", data.max_gx, data.max_gy, data.max_gz);

        Serial.printf("Sample Stats: total=%llu, dropped=%llu, SPS=%.2f\n",
                    data.total_samples, data.dropped_samples, data.samples_per_second);

        Serial.println("--- Rolling Max G Windows ---");
        for (const auto& [label, val] : data.max_g_windows_x) {
            float gy = data.max_g_windows_y.count(label) ? data.max_g_windows_y.at(label) : 0;
            float gz = data.max_g_windows_z.count(label) ? data.max_g_windows_z.at(label) : 0;

            Serial.printf(" [%s] Gx=%.3f, Gy=%.3f, Gz=%.3f\n", label.c_str(), val, gy, gz);
        }
        Serial.println("===========================");
    }

    /*
    void MPU6000::printMPUData(const mpu6000::data::MPUData& data) {
        Serial.println(F( "=== MPU6050 Sensor Data ===" ));
        Serial.print(F("Pitch: ")); Serial.print(data.pitch_deg); Serial.println(F("°"));
        Serial.print(F("Roll : ")); Serial.print(data.roll_deg); Serial.println(F("°"));
        
        Serial.print(F("Smoooooth G (gx_smooth, gy_smooth, gz_smooth): "));
        Serial.print(data.gx_smooth, 3); Serial.print(F(" | "));
        Serial.print(data.gy_smooth, 3); Serial.print(F(" | "));
        Serial.println(data.gz_smooth, 3);

        Serial.print(F("Max G Lifetime (gx, gy, gz): "));
        Serial.print(data.max_gx, 3); Serial.print(F(" | "));
        Serial.print(data.max_gy, 3); Serial.print(F(" | "));
        Serial.println(data.max_gz, 3);
        
         //Log.notice    (F(CR "******************************************" CR));
        Serial.println(F("\r\nRolling Max G (selected windows) [max (dir)]:"));
        auto print_window = [](const char* label, const data::GMaxWindow& w) {
            Serial.print(label); Serial.print(": ");
            Serial.print(w.max_gx, 3); Serial.print(" ("); Serial.print(w.dir_gx, 3); Serial.print(") / ");
            Serial.print(w.max_gy, 3); Serial.print(" ("); Serial.print(w.dir_gy, 3); Serial.print(") / ");
            Serial.print(w.max_gz, 3); Serial.print(" ("); Serial.print(w.dir_gz, 3); Serial.println(")");
        };

        
        Serial.println();
    }
  

    void MPU6000::printMPUData(const data::MPUData& data) {
        Log.infoln(F("=== IMU DATA REPORT ==="));
        Log.infoln(F("Pitch: %.2f deg, Roll: %.2f deg"), data.pitch_deg, data.roll_deg);
        Log.infoln(F("Raw G:  gx=%.3f, gy=%.3f, gz=%.3f"), data.gx, data.gy, data.gz);
        Log.infoln(F("Smooth G: gx=%.3f, gy=%.3f, gz=%.3f"), data.gx_smooth, data.gy_smooth, data.gz_smooth);
        
        Log.infoln(F("Lifetime Max G: gx=%.3f, gy=%.3f, gz=%.3f"), data.max_gx, data.max_gy, data.max_gz);
        
        Log.infoln(F("Sample Stats: total=%llu, dropped=%llu, SPS=%.2f"),
                data.total_samples, data.dropped_samples, data.samples_per_second);

        Log.infoln(F("--- Rolling Max G Windows ---"));
        for (const auto& [label, val] : data.max_g_windows_x) {
            float gy = data.max_g_windows_y.count(label) ? data.max_g_windows_y.at(label) : 0;
            float gz = data.max_g_windows_z.count(label) ? data.max_g_windows_z.at(label) : 0;

            Log.infoln(F(" [%s] Gx=%.3f, Gy=%.3f, Gz=%.3f"),
                    label.c_str(), val, gy, gz);
        }
        Log.infoln(F("==========================="));
    }
  */
    /*
    void MPU6000::smoothAndFilterMPUData(mpu6000::data::MPUData& data, float smoothing_alpha, float spike_threshold) {
        auto reject_spike = [&](float current, float last) {
            return abs(current - last) > spike_threshold;
        };

        data.pitch_deg_smooth = smoothing_alpha * data.pitch_deg + (1.0f - smoothing_alpha) * data.pitch_deg_smooth;
        data.roll_deg_smooth  = smoothing_alpha * data.roll_deg  + (1.0f - smoothing_alpha) * data.roll_deg_smooth;

        if (!reject_spike(data.gx, data.gx_last))
            data.gx_smooth = smoothing_alpha * data.gx + (1.0f - smoothing_alpha) * data.gx_smooth;
        data.gx_last = data.gx;

        if (!reject_spike(data.gy, data.gy_last))
            data.gy_smooth = smoothing_alpha * data.gy + (1.0f - smoothing_alpha) * data.gy_smooth;
        data.gy_last = data.gy;

        if (!reject_spike(data.gz, data.gz_last))
            data.gz_smooth = smoothing_alpha * data.gz + (1.0f - smoothing_alpha) * data.gz_smooth;
        data.gz_last = data.gz;
    }
    */
   void MPU6000::smoothAndFilterMPUData(data::MPUData& d, float alpha, float spike_thresh) {
        // Exponential moving average
        d.gx_smooth = alpha * d.gx + (1.0f - alpha) * d.gx_smooth;
        d.gy_smooth = alpha * d.gy + (1.0f - alpha) * d.gy_smooth;
        d.gz_smooth = alpha * d.gz + (1.0f - alpha) * d.gz_smooth;

        // Spike rejection
        if (abs(d.gx_smooth - d.gx) > spike_thresh) d.gx_smooth = d.gx;
        if (abs(d.gy_smooth - d.gy) > spike_thresh) d.gy_smooth = d.gy;
        if (abs(d.gz_smooth - d.gz) > spike_thresh) d.gz_smooth = d.gz;

        // Lifetime max tracking
        d.max_gx = std::max(d.max_gx, abs(d.gx_smooth));
        d.max_gy = std::max(d.max_gy, abs(d.gy_smooth));
        d.max_gz = std::max(d.max_gz, abs(d.gz_smooth));
        
        // Append to historical deque
        unsigned long now = millis();
        gx_history.push_back({now, abs(d.gx_smooth)});
        gy_history.push_back({now, abs(d.gy_smooth)});
        gz_history.push_back({now, abs(d.gz_smooth)});
        
        // Clear old samples outside the largest window
        auto cutoff = now - g_windows.back() * 1000;
        auto clean = [cutoff](auto& deque) {
            while (!deque.empty() && deque.front().first < cutoff) deque.pop_front();
        };
        clean(gx_history); clean(gy_history); clean(gz_history);
        
        // Compute per-window max
        d.max_g_windows_x.clear();
        d.max_g_windows_y.clear();
        d.max_g_windows_z.clear();

        for (auto window_sec : g_windows) {
            unsigned long window_start = now - window_sec * 1000;
            float max_x = 0.0f, max_y = 0.0f, max_z = 0.0f;

            for (const auto& [ts, val] : gx_history) if (ts >= window_start) max_x = std::max(max_x, val);
            for (const auto& [ts, val] : gy_history) if (ts >= window_start) max_y = std::max(max_y, val);
            for (const auto& [ts, val] : gz_history) if (ts >= window_start) max_z = std::max(max_z, val);

            String label = String(window_sec) + "s";
            d.max_g_windows_x[label] = max_x;
            d.max_g_windows_y[label] = max_y;
            d.max_g_windows_z[label] = max_z;
        }            
    }

    void MPU6000::update() {
        if (!initialized) return;
        unsigned long now = millis();        
        total_samples++; // Do sample stats update 
        int16_t ax, ay, az;
        int16_t gx_raw, gy_raw, gz_raw;
        
        mpu.getMotion6(&ax, &ay, &az, &gx_raw, &gy_raw, &gz_raw);

        _data.gx = gx_raw / 16384.0f;
        _data.gy = gy_raw / 16384.0f;
        _data.gz = gz_raw / 16384.0f;

        _data.pitch_deg = atan2(ax, sqrt(ay * ay + az * az)) * 180.0f / PI;
        _data.roll_deg  = atan2(ay, sqrt(ax * ax + az * az)) * 180.0f / PI;


        if (fabs(_data.gx) > fabs(_data.max_gx)) _data.max_gx = _data.gx;
        if (fabs(_data.gy) > fabs(_data.max_gy)) _data.max_gy = _data.gy;
        if (fabs(_data.gz) > fabs(_data.max_gz)) _data.max_gz = _data.gz;

        updateWindowMax(_data.max_1s, _data.gx, _data.gy, _data.gz, now, 1000);
        updateWindowMax(_data.max_5s, _data.gx, _data.gy, _data.gz, now, 5000);
        updateWindowMax(_data.max_10s, _data.gx, _data.gy, _data.gz, now, 10000);
        updateWindowMax(_data.max_15s, _data.gx, _data.gy, _data.gz, now, 15000);

        updateWindowMax(_data.max_30s, _data.gx, _data.gy, _data.gz, now, 30000);
        updateWindowMax(_data.max_45s, _data.gx, _data.gy, _data.gz, now, 45000);
        updateWindowMax(_data.max_60s, _data.gx, _data.gy, _data.gz, now, 60000);
        
        updateWindowMax(_data.max_1m, _data.gx, _data.gy, _data.gz, now, 60000);
        updateWindowMax(_data.max_5m, _data.gx, _data.gy, _data.gz, now, 300000);
        updateWindowMax(_data.max_10m, _data.gx, _data.gy, _data.gz, now, 600000);

        updateWindowMax(_data.max_15m, _data.gx, _data.gy, _data.gz, now, 900000);
        updateWindowMax(_data.max_30m, _data.gx, _data.gy, _data.gz, now, 1800000);      
        
        if (last_sample_time_ms > 0) {
            unsigned long delta = now - last_sample_time_ms;

            if (delta > 0) {
                float current_rate = 1000.0f / delta;

                // Optional exponential moving average smoothing
                float alpha = 0.05f;  // adjust to preference
                samples_per_second = (alpha * current_rate) + ((1.0f - alpha) * samples_per_second);
            } else {
                dropped_samples++;
            }
        }
        last_sample_time_ms = now;

        // Store into MPUData so the API can access it
        _data.total_samples = total_samples;
        _data.dropped_samples = dropped_samples;
        _data.samples_per_second = samples_per_second;
    }

    void MPU6000::setData(const data::MPUData& newData) {
        _data = newData;
    }

    data::MPUData MPU6000::getData() const {
        return _data;
    }

    void MPU6000::updateMax(float& max_val, float& dir_val, float new_val) {
        float abs_val = fabs(new_val);
        if (abs_val > max_val) {
            max_val = abs_val;
            dir_val = new_val;
        }
    }

    void MPU6000::updateWindowMax(data::GMaxWindow& win, float gx, float gy, float gz, unsigned long now, unsigned long duration_ms) {
        constexpr float alpha = 0.5f; // tunable smoothing factor (0.0–1.0)

        if (now - win.last_reset > duration_ms) {
            win.max_gx = fabs(gx); win.dir_gx = gx;
            win.max_gy = fabs(gy); win.dir_gy = gy;
            win.max_gz = fabs(gz); win.dir_gz = gz;
            win.last_reset = now;

            win.smooth_gx = win.max_gx;
            win.smooth_gy = win.max_gy;
            win.smooth_gz = win.max_gz;
        } else {
            updateMax(win.max_gx, win.dir_gx, gx);
            updateMax(win.max_gy, win.dir_gy, gy);
            updateMax(win.max_gz, win.dir_gz, gz);

            // Smooth using exponential moving average
            win.smooth_gx = alpha * win.max_gx + (1.0f - alpha) * win.smooth_gx;
            win.smooth_gy = alpha * win.max_gy + (1.0f - alpha) * win.smooth_gy;
            win.smooth_gz = alpha * win.max_gz + (1.0f - alpha) * win.smooth_gz;
        }
    }


} // namespace mpu6000

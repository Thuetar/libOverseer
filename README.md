# Overseer Library

https://randomnerdtutorials.com/esp32-s3-devkitc-pinout-guide/

# Abilities
## Read Water Level in Tanks
https://forums.raspberrypi.com/viewtopic.php?t=119614

## Current Sensing
* DC Current
    JESSINIE WCS1800 Hall Current Sensor Detector 35A Current Sensor Module Board Short Circuit Overcurrent Protection Module Sensor with Overcurrent Lamp
    WCS1800 Hall Current Sensor Detector 35A

* AC



## Device Notes,
i2c Devices
    Found device at 0x48 --- ADS111
    Found device at 0x68 --- MPU

SBD: NodeMCU 32S v1.1, also known as the ESP32S v1.1
https://www.espboards.dev/esp32/nodemcu-32s/


## Helpful Commands


pio run -t upload && pio device monitor


pio run --target cleanall && 
    rm -rf .pio/libdeps && 
    rm -rf .pio/build && 
    rm -rf .pio/.cache && 
    rm -rf ~/.platformio/lib 


pio run -t compiledb

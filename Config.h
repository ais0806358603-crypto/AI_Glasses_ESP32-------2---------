#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. WiFi & Cloud API Configuration
// ==========================================
extern const char* WIFI_SSID;
extern const char* WIFI_PASS;
extern const char* GCP_API_KEY; // Google Cloud Platform API Key (Vision & Speech)

// ==========================================
// 2. Hardware Pins Configuration (ESP32-S3 Sense)
// ==========================================
// I2S Speaker (MAX98357A)
#define I2S_SPK_BCLK    44
#define I2S_SPK_LRC     45
#define I2S_SPK_DOUT    43

// I2S Microphone (INMP441 or Built-in Digital Mic)
// Adjust these pins according to the exact ESP32-S3 Sense mic routing
#define I2S_MIC_SCK     41
#define I2S_MIC_WS      42
#define I2S_MIC_SD      43 // Just examples, check datasheet for exact S3 Sense mic pins. Assuming standard layout.

// Button & Switch
#define PIN_ACTION_BTN  4  // ปุ่มกดรับคำสั่ง
#define PIN_POWER_SW    15 // สวิตช์เปิด-ปิด (หรือใช้คั่นแบตเตอรี่เลยก็ได้ แต่ถ้ารับค่าเป็น Logic ใช้ Pin 15)

// Battery ADC
#define PIN_BATTERY_ADC 5  // ขาอ่านแรงดันแบตเตอรี่ (ผ่าน Voltage Divider)

// Camera (OV2640 on XIAO ESP32S3 Sense)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    10
#define SIOD_GPIO_NUM    40
#define SIOC_GPIO_NUM    39
#define Y9_GPIO_NUM      48
#define Y8_GPIO_NUM      11
#define Y7_GPIO_NUM      12
#define Y6_GPIO_NUM      14
#define Y5_GPIO_NUM      16
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM      17
#define Y2_GPIO_NUM      15
#define VSYNC_GPIO_NUM   38
#define HREF_GPIO_NUM    47
#define PCLK_GPIO_NUM    13

// ==========================================
// 3. System States & Constants
// ==========================================
enum SystemMode {
    MODE_IDLE,
    MODE_STANDBY,
    MODE_DETECTING,
    MODE_READING_OCR,
    MODE_DESCRIBE_SCENE,
    MODE_SEARCHING,
    MODE_LOW_POWER
};

enum DistanceLevel {
    DIST_SAFE,     // > 3m
    DIST_PREPARE,  // 1.5 - 3m
    DIST_WARNING,  // 0.5 - 1.5m
    DIST_DANGER    // 0 - 0.5m
};

#endif // CONFIG_H

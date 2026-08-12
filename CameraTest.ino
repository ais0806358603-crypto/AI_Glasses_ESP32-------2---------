#include "esp_camera.h"
#include <Arduino.h>

// ==========================================
// Camera Pins for XIAO ESP32S3 Sense
// ==========================================
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

bool cameraReady = false;

void setup() {
  Serial.begin(115200);
  delay(2000); // Wait for Serial Monitor to connect
  Serial.println();
  Serial.println("=================================");
  Serial.println("   XIAO ESP32S3 Sense Cam Test   ");
  Serial.println("=================================");

  // Configure Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;  // Start small (320x240) for stability
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init FAILED with error 0x%x\n", err);
    Serial.println("Camera will NOT work. Stopping here.");
    Serial.println("Check: Is the camera module plugged in firmly?");
    cameraReady = false;
    return; // Stop here, don't crash
  }

  Serial.println("Camera initialized successfully!");
  Serial.println("Starting capture test...\n");
  cameraReady = true;
}

void loop() {
  // If camera failed to init, just idle (no crash/reboot)
  if (!cameraReady) {
    delay(5000);
    return;
  }

  // Capture a frame
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("ERROR: Camera capture failed");
    delay(2000);
    return;
  }

  // Print frame details
  Serial.printf("OK! Size: %6u bytes | %ux%u\n", fb->len, fb->width, fb->height);

  // Return the frame buffer
  esp_camera_fb_return(fb);

  // Wait 2 seconds before next capture
  delay(2000);
}

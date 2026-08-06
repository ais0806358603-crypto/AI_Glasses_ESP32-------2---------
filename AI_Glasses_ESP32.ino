#include <Arduino.h>
#include "Config.h"
#include "BatteryMod.h"
#include "CameraMod.h"
#include "AudioMod.h"
#include "MicMod.h"
#include "CloudAPI.h"
#include "AppLogic.h"

// Define placeholders for Config.h externs
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* GCP_API_KEY = "YOUR_GOOGLE_CLOUD_API_KEY";

// System State
SystemMode currentMode = MODE_STANDBY;
String searchTarget = "";

// FreeRTOS Task Handles
TaskHandle_t TaskMainLoopHandle = NULL;
TaskHandle_t TaskBatteryHandle = NULL;

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing AI Glasses...");

    // Initialize Pins
    pinMode(PIN_ACTION_BTN, INPUT_PULLUP);
    pinMode(PIN_POWER_SW, INPUT_PULLUP);

    // Initialize Modules
    Battery.begin();
    Audio.begin();
    Mic.begin();
    
    // Connect to WiFi
    if (!Cloud.connectWiFi()) {
        Audio.playSystemSound("พบข้อผิดพลาด กรุณาตรวจสอบการเชื่อมต่อ");
    } else {
        // Init Camera last to save memory during WiFi init
        if (!Camera.begin()) {
            Audio.playSystemSound("พบข้อผิดพลาด กล้องไม่พร้อมใช้งาน");
        } else {
            Audio.playSystemSound("ระบบพร้อมใช้งาน");
            currentMode = MODE_DETECTING;
        }
    }

    // Create FreeRTOS Tasks
    xTaskCreatePinnedToCore(
        TaskBattery,      /* Task function. */
        "BatteryTask",    /* name of task. */
        4096,             /* Stack size of task */
        NULL,             /* parameter of the task */
        1,                /* priority of the task */
        &TaskBatteryHandle, /* Task handle to keep track of created task */
        0);               /* pin task to core 0 */                  

    xTaskCreatePinnedToCore(
        TaskMainLogic,    /* Task function. */
        "MainLogicTask",  /* name of task. */
        8192,             /* Stack size of task */
        NULL,             /* parameter of the task */
        2,                /* priority of the task */
        &TaskMainLoopHandle, /* Task handle to keep track of created task */
        1);               /* pin task to core 1 */
}

void loop() {
    // Empty, FreeRTOS handles the tasks
    delay(1000);
}

// ----------------------------------------------------------------
// TASK: Battery Monitoring
// ----------------------------------------------------------------
void TaskBattery(void *pvParameters) {
    for (;;) {
        Battery.update();
        if (Battery.isLowBattery()) {
            Cloud.textToSpeech(Battery.getStatusString());
            vTaskDelay(pdMS_TO_TICKS(60000)); // Alert every 1 minute if low
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // Check every 10 seconds
    }
}

// ----------------------------------------------------------------
// TASK: Main Logic (Camera, Button, Cloud, Audio)
// ----------------------------------------------------------------
void TaskMainLogic(void *pvParameters) {
    for (;;) {
        // Check Power Switch
        if (digitalRead(PIN_POWER_SW) == HIGH) { // Assuming HIGH is OFF
            if (currentMode != MODE_LOW_POWER) {
                currentMode = MODE_LOW_POWER;
                Audio.playSystemSound("ปิดระบบ");
                // In a real scenario, put ESP32 to Deep Sleep here
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // Check Action Button (Voice Command Trigger)
        if (digitalRead(PIN_ACTION_BTN) == LOW) {
            Audio.playSystemSound("กำลังรับฟัง");
            vTaskDelay(pdMS_TO_TICKS(500)); // Debounce
            
            // Record Audio
            uint8_t audioBuf[16000]; // 1 sec at 16kHz 16-bit (rough size, actually need 32KB, simplified for mock)
            size_t bytesRead = Mic.readAudio(audioBuf, sizeof(audioBuf));
            
            if (bytesRead > 0) {
                String command = Cloud.speechToText(audioBuf, bytesRead);
                processVoiceCommand(command);
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); // Debounce after action
            continue; // Skip camera frame this loop
        }

        // Perform actions based on current mode
        if (currentMode == MODE_DETECTING || currentMode == MODE_SEARCHING) {
            camera_fb_t* fb = Camera.captureFrame();
            if (fb) {
                String jsonRes = Cloud.analyzeImage(fb, false);
                Logic.processVisionResult(jsonRes, currentMode, searchTarget);
                Camera.releaseFrame(fb);
            }
            // Delay to avoid spamming Cloud API and blowing up quota
            vTaskDelay(pdMS_TO_TICKS(2000)); 
        } else if (currentMode == MODE_READING_OCR) {
            camera_fb_t* fb = Camera.captureFrame();
            if (fb) {
                String jsonRes = Cloud.analyzeImage(fb, true);
                Logic.processVisionResult(jsonRes, currentMode, "");
                Camera.releaseFrame(fb);
                
                // OCR is a one-shot action, return to detecting
                currentMode = MODE_DETECTING; 
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            vTaskDelay(pdMS_TO_TICKS(500)); // Standby
        }
    }
}

// ----------------------------------------------------------------
// Function: Process Voice Command
// ----------------------------------------------------------------
void processVoiceCommand(String cmd) {
    Serial.println("Command: " + cmd);
    
    if (cmd.indexOf("ปิดใช้งาน") != -1) {
        currentMode = MODE_STANDBY;
        Cloud.textToSpeech("เข้าสู่โหมดพัก");
    } 
    else if (cmd.indexOf("เปิดใช้งาน") != -1) {
        currentMode = MODE_DETECTING;
        Cloud.textToSpeech("ระบบพร้อมใช้งาน");
    }
    else if (cmd.indexOf("อ่านข้อความ") != -1) {
        currentMode = MODE_READING_OCR;
        Cloud.textToSpeech("กำลังอ่านข้อความ");
    }
    else if (cmd.indexOf("สถานะแบตเตอรี่") != -1 || cmd.indexOf("ช่วยเหลือ") != -1) {
        Cloud.textToSpeech(Battery.getStatusString());
    }
    else if (cmd.indexOf("หยุดแจ้งเตือน") != -1) {
        currentMode = MODE_STANDBY; // Temporary stop
        Cloud.textToSpeech("ปิดเสียงแจ้งเตือนชั่วคราว");
    }
    else if (cmd.indexOf("หา") == 0) { // Starts with "หา" (Find)
        searchTarget = cmd.substring(2); // Extract target
        searchTarget.trim();
        currentMode = MODE_SEARCHING;
        Cloud.textToSpeech("กำลังเข้าสู่โหมดค้นหา " + searchTarget);
    }
    else if (cmd.indexOf("ยกเลิกการค้นหา") != -1) {
        currentMode = MODE_DETECTING;
        searchTarget = "";
        Cloud.textToSpeech("กลับสู่โหมดปกติ");
    }
    else if (cmd.indexOf("อธิบายภาพ") != -1) {
        // Similar to OCR, one-shot action but for image captioning
        camera_fb_t* fb = Camera.captureFrame();
        if (fb) {
            // Mock captioning
            Cloud.textToSpeech("ภาพด้านหน้ามีโต๊ะและเก้าอี้");
            Camera.releaseFrame(fb);
        }
    }
}

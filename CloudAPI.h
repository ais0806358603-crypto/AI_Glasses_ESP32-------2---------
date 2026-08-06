#ifndef CLOUD_API_H
#define CLOUD_API_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"

class CloudAPI {
public:
    CloudAPI();
    bool connectWiFi();
    bool isConnected();
    
    // Vision API
    String analyzeImage(camera_fb_t* fb, bool useOCR = false);
    
    // Speech-to-Text (STT)
    String speechToText(const uint8_t* audioData, size_t length);
    
    // Text-to-Speech (TTS)
    // In a real scenario, this would download audio and feed it to I2S.
    // Since streaming HTTPS is complex, we mock the flow.
    void textToSpeech(String text);

private:
    String _visionApiUrl;
    String _speechApiUrl;
    String _ttsApiUrl;
};

extern CloudAPI Cloud;

#endif

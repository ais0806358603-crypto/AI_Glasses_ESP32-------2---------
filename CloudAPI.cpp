#include "CloudAPI.h"
#include "Config.h"
#include "AudioMod.h"

CloudAPI Cloud;

CloudAPI::CloudAPI() {
    _visionApiUrl = "https://vision.googleapis.com/v1/images:annotate?key=" + String(GCP_API_KEY);
    _speechApiUrl = "https://speech.googleapis.com/v1/speech:recognize?key=" + String(GCP_API_KEY);
    _ttsApiUrl = "https://texttospeech.googleapis.com/v1/text:synthesize?key=" + String(GCP_API_KEY);
}

bool CloudAPI::connectWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(500);
        Serial.print(".");
        retries++;
    }
    Serial.println("");
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    Serial.println("WiFi Connection Failed.");
    return false;
}

bool CloudAPI::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String CloudAPI::analyzeImage(camera_fb_t* fb, bool useOCR) {
    if (!isConnected()) return "No Internet";
    
    Serial.println("Sending image to Cloud API...");
    // Mocking the HTTP Post logic due to complexity of Base64 encoding a full JPEG in memory.
    // In actual implementation, you would:
    // 1. Base64 encode fb->buf (size: fb->len)
    // 2. Build JSON payload for Google Vision (TEXT_DETECTION or OBJECT_LOCALIZATION)
    // 3. POST to _visionApiUrl
    // 4. Parse the response
    
    delay(1500); // Simulate network latency
    
    if (useOCR) {
        return "ทดสอบระบบอ่านข้อความภาษาไทย";
    } else {
        // Return a mock JSON string containing objects and bounding boxes
        return "{\"objects\":[{\"name\":\"Person\",\"score\":0.95,\"bbox\":[0.2,0.1,0.5,0.8]}, {\"name\":\"Chair\",\"score\":0.88,\"bbox\":[0.6,0.5,0.9,0.9]}]}";
    }
}

String CloudAPI::speechToText(const uint8_t* audioData, size_t length) {
    if (!isConnected()) return "";
    
    Serial.println("Sending audio to STT API...");
    // Mock implementation
    // 1. Base64 encode audioData
    // 2. Build JSON with config (languageCode: 'th-TH', sampleRateHertz: 16000)
    // 3. POST to _speechApiUrl
    
    delay(1000); // Simulate network latency
    
    // Simulate detecting a keyword
    return "ตรวจข้างหน้า"; 
}

void CloudAPI::textToSpeech(String text) {
    if (!isConnected()) return;
    
    Serial.println("Requesting TTS for: " + text);
    // Mock implementation
    // 1. Build JSON with text, voice (th-TH-Standard-A)
    // 2. POST to _ttsApiUrl
    // 3. Receive Base64 encoded audio content
    // 4. Decode Base64 and feed to Audio.feedTTSData()
    
    delay(500);
    // Simulate playing audio
    Audio.playSystemSound("Simulated TTS Audio for: " + text);
}

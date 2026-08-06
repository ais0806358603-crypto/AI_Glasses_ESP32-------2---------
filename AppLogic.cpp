#include "AppLogic.h"
#include "CloudAPI.h"
#include "AudioMod.h"

AppLogic Logic;

AppLogic::AppLogic() {
}

DistanceLevel AppLogic::estimateDistance(float bbox[4], String objectName) {
    // bbox array: [x_min, y_min, x_max, y_max]
    float height = bbox[3] - bbox[1];
    
    // This is a VERY rough estimation.
    // If a person takes up 80% of the frame height, they are very close.
    // Different objects have different real-world heights.
    float dangerThreshold = 0.7; // > 70% of frame = Danger (0-0.5m)
    float warningThreshold = 0.4; // > 40% of frame = Warning (0.5-1.5m)
    float prepareThreshold = 0.15; // > 15% of frame = Prepare (1.5-3.0m)
    
    if (height > dangerThreshold) return DIST_DANGER;
    if (height > warningThreshold) return DIST_WARNING;
    if (height > prepareThreshold) return DIST_PREPARE;
    
    return DIST_SAFE;
}

int AppLogic::getPriority(const DetectedObject& obj) {
    // 1. Moving objects (Person, Cat, Dog, Car)
    if (obj.name == "Person" || obj.name == "Cat" || obj.name == "Dog" || obj.name == "Car" || obj.name == "Bicycle") {
        return 1; // Highest
    }
    // 2. Danger objects (Stairs) - Note: Google Vision might not detect stairs reliably as objects, sometimes labels.
    if (obj.name == "Stairs") {
        return 2;
    }
    // 3. Nearest
    if (obj.distanceLevel == DIST_DANGER) return 3;
    if (obj.distanceLevel == DIST_WARNING) return 4;
    
    return 5; // General
}

bool AppLogic::shouldAlert(DetectedObject& obj) {
    // Do not alert if > 3m (except moving, but we simplify here)
    if (obj.distanceLevel == DIST_SAFE) return false;
    
    // Check history (8 seconds rule)
    unsigned long now = millis();
    for (int i = 0; i < _historyCount; i++) {
        if (_history[i].name == obj.name) {
            // Found in history
            if (now - _history[i].lastDetectedTime < 8000) {
                // Within 8 seconds. Alert ONLY if distance level got more severe
                if (obj.distanceLevel < _history[i].distanceLevel) { // Lower enum value means closer
                    _history[i].distanceLevel = obj.distanceLevel;
                    _history[i].lastDetectedTime = now;
                    return true;
                }
                return false; // Skip alert
            } else {
                // Past 8 seconds, update time and alert
                _history[i].lastDetectedTime = now;
                _history[i].distanceLevel = obj.distanceLevel;
                return true;
            }
        }
    }
    
    // Not in history, add it
    if (_historyCount < 10) {
        obj.lastDetectedTime = now;
        _history[_historyCount++] = obj;
    }
    return true;
}

String AppLogic::translateObjectName(String englishName) {
    // Basic translation dictionary
    if (englishName == "Person") return "คน";
    if (englishName == "Cat") return "แมว";
    if (englishName == "Dog") return "สุนัข";
    if (englishName == "Car") return "รถยนต์";
    if (englishName == "Bicycle") return "จักรยาน";
    if (englishName == "Chair") return "เก้าอี้";
    if (englishName == "Table" || englishName == "Desk") return "โต๊ะ";
    if (englishName == "Door") return "ประตู";
    if (englishName == "Stairs") return "บันได";
    if (englishName == "Pillar") return "เสา";
    if (englishName == "Wall") return "กำแพง";
    return englishName; // Fallback
}

void AppLogic::processVisionResult(String jsonResult, SystemMode currentMode, String targetObject) {
    if (currentMode == MODE_READING_OCR) {
        Cloud.textToSpeech(jsonResult); // For OCR, result is plain text
        return;
    }

    // Parse JSON
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonResult);
    if (error) {
        Serial.println("JSON Parse Error");
        return;
    }

    JsonArray objects = doc["objects"];
    if (objects.size() == 0) return;

    DetectedObject detectedList[10];
    int detectedCount = 0;

    for (JsonObject obj : objects) {
        if (detectedCount >= 10) break;
        
        DetectedObject dObj;
        dObj.name = obj["name"].as<String>();
        dObj.score = obj["score"].as<float>();
        
        JsonArray bbox = obj["bbox"];
        dObj.bbox[0] = bbox[0].as<float>();
        dObj.bbox[1] = bbox[1].as<float>();
        dObj.bbox[2] = bbox[2].as<float>();
        dObj.bbox[3] = bbox[3].as<float>();
        
        dObj.distanceLevel = estimateDistance(dObj.bbox, dObj.name);
        detectedList[detectedCount++] = dObj;
    }

    // Handle Searching Mode
    if (currentMode == MODE_SEARCHING && targetObject != "") {
        bool found = false;
        for (int i = 0; i < detectedCount; i++) {
            if (translateObjectName(detectedList[i].name) == targetObject || detectedList[i].name == targetObject) {
                found = true;
                // Calculate position based on bbox X center
                float xCenter = (detectedList[i].bbox[0] + detectedList[i].bbox[2]) / 2.0;
                String posStr = "ตรงหน้า";
                if (xCenter < 0.33) posStr = "ด้านซ้าย";
                else if (xCenter > 0.66) posStr = "ด้านขวา";
                
                Cloud.textToSpeech(targetObject + " อยู่" + posStr);
                break;
            }
        }
        if (!found) {
            Cloud.textToSpeech("กำลังค้นหา");
        }
        return;
    }

    // Normal Detection Mode
    // 1. Filter out SAFE distance (unless it's moving, but we skip for now)
    // 2. Sort by Priority
    // A simple bubble sort for priorities (lower number = higher priority)
    for (int i = 0; i < detectedCount - 1; i++) {
        for (int j = 0; j < detectedCount - i - 1; j++) {
            if (getPriority(detectedList[j]) > getPriority(detectedList[j+1])) {
                DetectedObject temp = detectedList[j];
                detectedList[j] = detectedList[j+1];
                detectedList[j+1] = temp;
            }
        }
    }

    // 3. Alert based on priority and history
    for (int i = 0; i < detectedCount; i++) {
        if (shouldAlert(detectedList[i])) {
            String thaiName = translateObjectName(detectedList[i].name);
            String alertMsg = "";
            
            if (detectedList[i].distanceLevel == DIST_DANGER) {
                alertMsg = "หยุด ระวัง มี" + thaiName + "อยู่ตรงนี้";
            } else if (detectedList[i].distanceLevel == DIST_WARNING) {
                alertMsg = "มี" + thaiName + "อยู่ตรงหน้า";
            } else if (detectedList[i].distanceLevel == DIST_PREPARE) {
                // Only alert prepare for important things
                if (getPriority(detectedList[i]) <= 2) {
                    alertMsg = "ระวัง " + thaiName + "ข้างหน้า";
                }
            }
            
            if (alertMsg != "") {
                Cloud.textToSpeech(alertMsg);
                break; // Only announce the highest priority item per frame to avoid overwhelming
            }
        }
    }
}

void AppLogic::handleVoiceCommand(String command) {
    // This function will set the state machine in Main.ino
    // Since Main.ino controls the state, we can return the command type or use a callback.
    // For simplicity, we just print here, and in Main.ino we will check the command string.
    Serial.println("Parsed Command: " + command);
}

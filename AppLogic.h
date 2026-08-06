#ifndef APP_LOGIC_H
#define APP_LOGIC_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"

struct DetectedObject {
    String name;
    float score;
    float bbox[4]; // [x_min, y_min, x_max, y_max] normalized 0.0-1.0
    DistanceLevel distanceLevel;
    unsigned long lastDetectedTime;
    bool isDanger;
    bool isMoving; // Hard to determine from single frame, will mock or infer from tracking
};

class AppLogic {
public:
    AppLogic();
    void processVisionResult(String jsonResult, SystemMode currentMode, String targetObject = "");
    
    // Command parser
    void handleVoiceCommand(String command);

private:
    DistanceLevel estimateDistance(float bbox[4], String objectName);
    int getPriority(const DetectedObject& obj);
    bool shouldAlert(DetectedObject& obj);
    String translateObjectName(String englishName);
    
    DetectedObject _history[10]; // Keep track of recently alerted objects
    int _historyCount = 0;
};

extern AppLogic Logic;

#endif

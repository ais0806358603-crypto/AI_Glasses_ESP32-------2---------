#ifndef CAMERA_MOD_H
#define CAMERA_MOD_H

#include <Arduino.h>
#include "esp_camera.h"
#include "Config.h"

class CameraMod {
public:
    CameraMod();
    bool begin();
    camera_fb_t* captureFrame();
    void releaseFrame(camera_fb_t* fb);
    void end();
};

extern CameraMod Camera;

#endif

// Camera.h - a Blender inspired OF camera
// author: @kikko_fr

#pragma once

#include "ofMain.h"

class Camera : public ofCamera {

public:
    
    Camera();
    virtual ~Camera();
    
    void setup();
    void update();
    
    void reset();
    
    ofVec3f target;
    float longitude, latitude, distance;
    float speed;
    
    void setInteractive(bool bInteractive);
    
    void storePosition();
    void restorePosition();
    
    typedef Camera* Ptr;
    Ptr ptr() { return this; }
    
protected:
    
    void mousePressed(ofMouseEventArgs& ev);
    void mouseDragged(ofMouseEventArgs& ev);
    void mouseReleased(ofMouseEventArgs& ev);
    
    void addListeners();
    void removeListeners();
    
    float orbitSensivity;
    float panSensivity;
    float dollySensivity;
    
    ofVec3f initMouse;
    
    ofVec3f i_target, c_target, s_target;
    float   i_dist,   c_dist,   s_dist;
    float   i_long,   c_long,   s_long;
    float   i_lat,    c_lat,    s_lat;
};

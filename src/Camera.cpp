#include "Camera.h"

Camera::Camera() {
    reset();
}

Camera::~Camera() {
    removeListeners();
}

void Camera::reset() {
    orbitSensivity = 0.2;
    panSensivity   = 1.0;
    dollySensivity = 2.0;
    speed = 0.2;
    
    longitude = i_long   = c_long   = -180;
    latitude  = i_lat    = c_lat    = -20;
    distance  = i_dist   = c_dist   = 300;
    target    = i_target = c_target = ofPoint(0,0,300);
}

void Camera::setup() {
    
    setFov(70.6f);
    setFarClip(20000.f);
    
    addListeners();
}

void Camera::storePosition(){
    s_target = c_target;
    s_long   = c_long;
    s_lat    = c_lat;
    s_dist   = c_dist;
}

void Camera::restorePosition(){
    target     = s_target;
    longitude  = s_long;
    latitude   = s_lat;
    distance   = s_dist;
}

void Camera::setInteractive(bool bInteractive){
    if(bInteractive){
        addListeners();
    } else {
        removeListeners();
    }
}

void Camera::addListeners(){
    ofAddListener(ofEvents().mousePressed, this, &Camera::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &Camera::mouseReleased);
}

void Camera::removeListeners(){
    ofRemoveListener(ofEvents().mousePressed, this, &Camera::mousePressed);
    ofRemoveListener(ofEvents().mouseReleased, this, &Camera::mouseReleased);
}

void Camera::mousePressed(ofMouseEventArgs& ev) {
    initMouse  = ofPoint(ofGetMouseX(), ofGetMouseY());
    i_long   = longitude;
    i_lat    = latitude;
    i_dist   = distance;
    i_target = target;
    ofAddListener(ofEvents().mouseDragged, this, &Camera::mouseDragged);
}

void Camera::mouseDragged(ofMouseEventArgs& ev) {
    
    if(ofGetKeyPressed(OF_KEY_SHIFT)) {
        float amountX = (ofGetMouseX() - initMouse.x) * panSensivity;
        float amountY = (ofGetMouseY() - initMouse.y) * panSensivity;
        ofNode node;
        node.setPosition(i_target);
        node.move(-getXAxis() *  amountX);
        node.move(getYAxis() *  amountY);
        target.set(node.getPosition());
    }
    else if(ofGetKeyPressed(OF_KEY_CONTROL)) {
        distance = i_dist - (ofGetMouseY() - initMouse.y) * dollySensivity;
        distance = fmax(10.f, distance);
    }
    else {
        longitude = i_long - (ofGetMouseX() - initMouse.x) * orbitSensivity;
        latitude  = i_lat  - (ofGetMouseY() - initMouse.y) * orbitSensivity;
    }
}

void Camera::mouseReleased(ofMouseEventArgs& ev) {
    ofRemoveListener(ofEvents().mouseDragged, this, &Camera::mouseDragged);
}

void Camera::update() {
    c_long   += (longitude - c_long)   * speed;
    c_lat    += (latitude  - c_lat)    * speed;
    c_dist   += (distance  - c_dist)   * speed;
    c_target += (target    - c_target) * speed;
    orbit(c_long, c_lat, c_dist, c_target);
}
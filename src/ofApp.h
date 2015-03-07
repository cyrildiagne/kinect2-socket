#pragma once

//#define HEADLESS

#include "ofMain.h"
#include "ofxKinectForWindows2.h"
#include "Sockets.h"

#ifdef HEADLESS
 #include "ofxXmlSettings.h"
#else
 #include "Camera.h"
 #include "ofxUI.h"
#endif

typedef ofxKinectForWindows2::Source::Depth::PointCloudOptions PointCloudOptions;

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void exit();

	void sendBodyDataJSON();
	string getBodyJSON(ofxKinectForWindows2::Body & body);

	ofxKFW2::Device kinect;
	ofxKFW2::Source::VGB vgb;

	PointCloudOptions pointCloudOpts;

	Sockets * sockets;
	bool bSendData;
	bool bSingleUserMode;

	// single user mode utils

	UINT64 updateMostLegitimateBody();
	UINT64 getMostCenteredBodyId();

	UINT64	currLegitimateTrackingId;
	UINT64	newcomerId;
	int		newcomerNumPresence;
	int		newcomerNumFrameForElection;

#ifdef HEADLESS
	ofxXmlSettings settings;
#else
	void draw();
	void drawMesh();
	void drawFloor();
	void drawFrustum();
	void drawBodies();
	void drawSensorPosition();
	void drawFaceFeatures();
	void drawGestures();

	ofMesh mesh;
	Camera camera;

	bool bDraw3D;
	bool bDrawMesh;
	bool bDrawSensorDebug;
	float meshFillOpacity;

	ofxUICanvas *gui;
	void setupGui();
#endif
};

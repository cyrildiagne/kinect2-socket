#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

#ifdef HEADLESS
	settings.load("settings.xml");
#else
	ofBackground(40);
	ofSetVerticalSync(true);
	ofDisableAntiAliasing();

	camera.setup();

	pointCloudOpts.steps = 4;
	pointCloudOpts.textureCoordinates = PointCloudOptions::TextureCoordinates::ColorCamera;

	bDraw3D = false;
	bDrawMesh = false;
	bDrawSensorDebug = false;
	meshFillOpacity = 100.f;

	gui = new ofxUICanvas();
	setupGui();
#endif

	ofSetFrameRate(31);

	//ofSetLogLevel("ofxLibwebsockets", OF_LOG_VERBOSE);
	//ofSetLogLevel("Sockets", OF_LOG_VERBOSE);
	//ofSetLogLevel("VGB", OF_LOG_VERBOSE);

	kinect.open();
	kinect.initDepth();
	kinect.initColor();
	kinect.initInfrared();
	kinect.initBodyIndex();
	kinect.initBodyFrame();

	//vgb.init(kinect.getSensor(), "sap.gbd", 0);

	bSendData = true;
	bSingleUserMode = true;

	currLegitimateTrackingId = 0;
	newcomerId = 0;
	newcomerNumPresence = 0;
	newcomerNumFrameForElection = 30;

	sockets = new Sockets();
	sockets->setup();
}

void ofApp::exit(){
	sockets->close();
	// TODO: fix MutexImpl::~MutexImpl() crash in POCO vs64 libs
	// and clean socket threads on exit :
	// delete sockets;
#ifndef HEADLESS
	gui->saveSettings("settings.xml");
	delete gui;
#endif
}

#ifndef HEADLESS
void ofApp::setupGui(){
	gui->addLabel("KINECT SOCKET", OFX_UI_FONT_MEDIUM);
	gui->addToggle("3D view", &bDraw3D);
	gui->addToggle("draw sensor debug", &bDrawSensorDebug);
	gui->addToggle("draw mesh", &bDrawMesh);
	gui->addSlider("mesh fill opacity", 0.f, 255.f, &meshFillOpacity);
	gui->addIntSlider("mesh steps", 1, 12, &pointCloudOpts.steps);
	gui->addToggle("single user mode", &bSingleUserMode);
	gui->addToggle("send body data", &bSendData);
	gui->autoSizeToFitWidgets() ;
	gui->loadSettings("settings.xml");
}
#endif

//--------------------------------------------------------------

void ofApp::update(){

	kinect.update();

	for (auto &b : kinect.getBodyFrame()->getBodies()) {
		if (!b.tracked) {
			if (b.isTrackingHands()){
				b.setTrackHands(false);
			}
			if (b.isTrackingFaceProperties()){
				b.setTrackFaceProperties(false);
			}
			/*if (vgb.getBodyId() == b.trackingId) {
				vgb.setBodyId(0);
			}*/
			continue;
		};

		if (!bSingleUserMode) {
			b.setTrackHands(true);
			b.setTrackFaceProperties(true);
		}
	}

	//vgb.update();

	if (bSingleUserMode) {

		updateMostLegitimateBody();
		auto * b = kinect.getBodyFrame()->getBodyPtrById(currLegitimateTrackingId);
		if (b) {
			b->setTrackFaceProperties(true);
			b->setTrackHands(true);
		}
	}

	if (bSendData) sendBodyDataJSON();
}


//--------------------------------------------------------------

UINT64 ofApp::getMostCenteredBodyId() {

	auto & bodies = kinect.getBodyFrame()->getBodies();

	UINT64 closestToCenter = 0;
	float distanceToCenter = 99999.f;
	for (int i = 0; i < bodies.size(); i++)
	{
		if (!bodies[i].tracked) continue;
		float spineDistXFromCenter = abs(bodies[i].joints[JointType_SpineBase].getPosition().x);
		if (spineDistXFromCenter < distanceToCenter) {
			closestToCenter = bodies[i].trackingId;
			distanceToCenter = spineDistXFromCenter;
		}
	}
	return closestToCenter;
}

UINT64 ofApp::updateMostLegitimateBody() {
	
	UINT64 legitimate = getMostCenteredBodyId();
	if (legitimate == 0) {
		return 0;
	}

	if (legitimate != currLegitimateTrackingId) {
		if (legitimate != newcomerId) {
			ofLog() << "New Comer " << legitimate;
			newcomerId = legitimate;
			newcomerNumPresence = 0;
		}
		if (newcomerNumPresence++ > newcomerNumFrameForElection) {
			ofLog() << "New legitimate " << legitimate;
			currLegitimateTrackingId = legitimate;
			newcomerId = 0;
			newcomerNumPresence = 0;
			//vgb.setBodyId(currLegitimateTrackingId);
		}
	}
	else {
		if (newcomerNumPresence > 0) {
			ofLog() << "Reset new comer presence " << legitimate;
			newcomerNumPresence = 0;
		}
	}

	if (currLegitimateTrackingId != 0) {
		return currLegitimateTrackingId;
	}
	return 0;
}


//--------------------------------------------------------------
#define _jsonp(P,V) "\"" P "\":" << V
#define _jsonps(P,V) "\"" P "\":" << "\"" << V << "\""

void ofApp::sendBodyDataJSON(){

	ostringstream buff;
	buff << "{";
	buff << "\"bodies\":[";

	if (bSingleUserMode) {
		if (currLegitimateTrackingId != 0) {
			auto * b = kinect.getBodyFrame()->getBodyPtrById(currLegitimateTrackingId);
			if (b && b->tracked) {
				buff << getBodyJSON(*b);
			}
		}
	}
	else {
		for (auto & b : kinect.getBodyFrame()->getBodies()) {
			if (!b.tracked) continue;
			buff << getBodyJSON(b);
		}
	}

	buff << "]}";

	string val = buff.str();
	ofStringReplace(val, ",]", "]");
	ofStringReplace(val, ",}", "}");
	sockets->send(val);
}

string ofApp::getBodyJSON(ofxKinectForWindows2::Body & b){

	const string handStates[] = { "unknown", "nottracked", "open", "closed", "lasso" };
	const string prop_names[] = { "Happy", "Engaged", "WearingGlasses", "LeftEyeClosed", "RightEyeClosed", "MouthOpen", "MouthMoved", "LookingAway" };
	const string results[]	  = { "unknown", "no", "maybe", "yes" };

	ostringstream buff;
	buff << "{";
	buff << _jsonp("id", b.trackingId) << ",";
	if (b.isTrackingHands()){
		buff << _jsonps("leftHandState",  handStates[b.leftHandState]) << ",";
		buff << _jsonps("rightHandState", handStates[b.rightHandState]) << ",";
	}
	if (b.isTrackingFaceProperties()) {
		buff << "\"face\":{";
		for (int i = 0; i < FaceProperty_Count; i++){
			DetectionResult res = b.faceProperties[static_cast<FaceProperty>(i)];
			if (res != DetectionResult_Unknown) {
				buff << "\"" << prop_names[i] << "\":\"" << results[res] << "\",";
			}
		}
		buff << "},";
	}
	buff << "\"gestures\":{";/*
	for (auto & g : vgb.getGestures()) {
		buff << "\"" << g->name << "\":{";
		if (g->type == GestureType_Discrete) {
			buff << _jsonps("type", "discrete") << ",";
			buff << _jsonp("detected", (g->detected?1:0)) << ",";
			buff << _jsonp("confidence", g->confidence);
		}
		else if (g->type == GestureType_Continuous) {
			buff << _jsonps("type", "continuous") << ",";
			buff << _jsonp("progress", g->progress);
		}
		buff << "},";
	}*/
	buff << "},";
	buff << "\"joints\":[";
	for (auto & j : b.joints) {
		const JointType & type = j.first;
		const ofVec3f & pos = j.second.getPosition();
		buff << "{";
		buff << _jsonp("x", pos.x) << ",";
		buff << _jsonp("y", pos.y) << ",";
		buff << _jsonp("z", pos.z) << "},";
	}
	buff << "]},";
	return buff.str();
}

#undef _jsonp
//--------------------------------------------------------------

#ifndef HEADLESS

void ofApp::draw(){

	ofSetWindowTitle(ofToString(ofGetFrameRate(), 1) + "fps"); 
	ofSetColor(255);
	if (bDraw3D) {

		mesh = kinect.getDepth()->getMesh(pointCloudOpts);

		ofEnableDepthTest();

		camera.update();
		camera.begin();

		ofPushMatrix();
		ofScale(100, 100, 100);
		{
			drawFloor();
			if (bDrawSensorDebug) drawSensorPosition();

			ofPushMatrix();
			ofMultMatrix(kinect.getBodyFrame()->getFloorTransform());
			{ 
				if (bDrawMesh) drawMesh();
				drawBodies();
				if (bDrawSensorDebug) drawFrustum();
			}
			ofPopMatrix();
		}
		ofPopMatrix();

		camera.end();

		ofDisableDepthTest();
	}
	else {
		kinect.getColor()->draw(0, 0, ofGetWidth(), ofGetHeight());
		kinect.getBodyFrame()->drawProjected(0, 0, ofGetWidth(), ofGetHeight());
	}
	drawFaceFeatures();
	drawGestures();
}

void ofApp::drawFloor() {
	ofSetColor(150, 150, 150);
	ofPushMatrix();
	ofRotate(90, 0, 0, -1);
	ofDrawGridPlane(10, 10);
	ofPopMatrix();
}

void ofApp::drawBodies() {
	ofDisableDepthTest();
	ofPushStyle();
	auto & bonesDef = kinect.getBodyFrame()->getBonesDef();
	auto & bodies = kinect.getBodyFrame()->getBodies();
	for (auto & b : bodies) {
		if (!b.tracked) continue;
		ofSetColor(0, 255, 0);
		for (auto & j : b.joints) {
			const ofVec3f & p = j.second.getPosition();
			ofDrawSphere(p, 0.02);
		}
		ofSetColor(255, 255, 0);
		for (auto & bdef : bonesDef) {
			if (b.joints.count(bdef.first) && b.joints.count(bdef.second)) {
				auto & j1 = b.joints.at(bdef.first);
				auto & j2 = b.joints.at(bdef.second);
				ofLine(j1.getPosition(), j2.getPosition());
			}
		}
	}
	ofPopStyle();
	ofDisableDepthTest();
}

void ofApp::drawMesh() {

	ofPushStyle();

	kinect.getColor()->getTextureReference().bind();

	ofSetColor(255, meshFillOpacity);
	mesh.drawFaces();

	kinect.getColor()->getTextureReference().unbind();

	ofPopStyle();
}

void ofApp::drawFrustum() {
	
	ofPushMatrix();
	ofScale(10, 10, 10);

	//draw the view cones of depth and colour cameras
	ofPushStyle();
	ofNoFill();
	ofSetLineWidth(2.0f);
	ofSetColor(100, 200, 100);
	kinect.getDepth()->drawFrustum();
	ofSetColor(200, 100, 100);
	kinect.getColor()->drawFrustum();
	ofPopStyle();

	ofPopMatrix();
}

void ofApp::drawSensorPosition() {

	ofPushStyle();
	ofSetColor(255, 255, 0);
	const ofMatrix4x4 & mat = kinect.getBodyFrame()->getFloorTransform();
	float sensorYcm = mat.getTranslation().y;
	ofLine(0, 0, 0, sensorYcm);
	ofDrawBitmapString(" " + ofToString(sensorYcm * 100, 2) + "cm", 0.1, sensorYcm*0.5, 0);
	float sensorXrot = mat.getRotate().getEuler().z;
	ofDrawBitmapString(" " + ofToString(sensorXrot * 100, 2) + "deg", 0.1, sensorYcm, 0);
	ofPopStyle();
}

void ofApp::drawFaceFeatures() {

	ofPushStyle();
	ofSetColor(ofColor::yellow);

	string results[] = { "unknown", "no", "maybe", "yes" };
	string props[] = { "Happy", "Engaged", "WearingGlasses", "LeftEyeClosed", "RightEyeClosed", "MouthOpen", "MouthMoved", "LookingAway" };
	string buff = "";
	for (auto &b : kinect.getBodyFrame()->getBodies()) {
		if (!b.tracked) continue;
		if (b.isTrackingFaceProperties()) {
			for (int i = 0; i < FaceProperty_Count; i++)
			{
				string prop = results[b.faceProperties[(FaceProperty)i]];
				buff += props[i] + ": " + prop + "\n";
			}
			ofDrawBitmapStringHighlight(buff, 230, 20);
		}
	}

	ofPopStyle();
}

void ofApp::drawGestures() {

	ofPushStyle();
	ofSetColor(ofColor::yellow);

	//string buff = vgb.getDebugString();
	//ofDrawBitmapStringHighlight(buff, 430, 20);

	ofPopStyle();
}

#endif
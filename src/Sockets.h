// Sockets.h
// author: @kikko_fr

#pragma once

#include "ofxLibwebsockets.h"

class Sockets
{
public:
	Sockets() {}
	virtual ~Sockets() {}

	void setup();
	void close();

	void send(string data);
	
	// websocket methods
	void onConnect(ofxLibwebsockets::Event& args);
	void onOpen(ofxLibwebsockets::Event& args);
	void onClose(ofxLibwebsockets::Event& args);
	void onIdle(ofxLibwebsockets::Event& args);
	void onMessage(ofxLibwebsockets::Event& args);
	void onBroadcast(ofxLibwebsockets::Event& args);
	
private:
	ofxLibwebsockets::Server server;
};


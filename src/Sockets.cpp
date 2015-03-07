#include "Sockets.h"

void Sockets::setup(){
	
	ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
	options.port = 9092;
	options.bUseSSL = false;
	//options.bBinaryProtocol = true;
	server.setup(options);
	server.addListener(this);
}


void Sockets::close(){
	server.close();
}

void Sockets::send(string data){
	server.send(data);
}

//--------------------------------------------------------------
void Sockets::onConnect(ofxLibwebsockets::Event& args){
	ofLogVerbose("Sockets") << "Connected";
}

//--------------------------------------------------------------
void Sockets::onOpen(ofxLibwebsockets::Event& args){
	ofLogVerbose("Sockets") << "New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName();
	// send welcome message on connection
	// args.conn.send("hi there");
}

//--------------------------------------------------------------
void Sockets::onClose(ofxLibwebsockets::Event& args){
	ofLogVerbose("Sockets") << "Connection closed";
}

//--------------------------------------------------------------
void Sockets::onIdle(ofxLibwebsockets::Event& args){
	ofLogVerbose("Sockets") << "Idle";
}

//--------------------------------------------------------------
void Sockets::onMessage(ofxLibwebsockets::Event& args){

	// trace out string messages or JSON messages!
	if (!args.json.isNull()){
		ofLogVerbose("Sockets") << "New message: " + args.json.toStyledString() + " from " + args.conn.getClientName();
	}
	else {
		ofLogVerbose("Sockets") << "New message: " + args.message + " from " + args.conn.getClientName();
	}
}

//--------------------------------------------------------------
void Sockets::onBroadcast(ofxLibwebsockets::Event& args){
	ofLogVerbose("Sockets") << "got broadcast " << args.message;
}

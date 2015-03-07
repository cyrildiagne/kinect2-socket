#include "ofMain.h"
#include "ofApp.h"
#include "ofAppNoWindow.h"

//========================================================================
int main(){

#ifdef HEADLESS
	ofAppNoWindow w;
	ofSetWindow(&w);
#else
	ofPtr<ofBaseRenderer> renderer(new ofGLProgrammableRenderer(true));
	ofSetCurrentRenderer(renderer);
	ofSetupOpenGL(853, 480, OF_WINDOW);			// <-------- setup the GL context
#endif

	ofRunApp(new ofApp());
}

#pragma once

#include "ofMain.h"

#include "ofxSimpleApp.h"
//#include "ofxSimpleAppUtils.h"

class ofApp : public ofxSimpleApp {

	public:
        ofApp(){};
        ~ofApp() override {};

        void setup() override;
        void exit() override;
        void update() override;
        void draw() override;
        void drawScene() override;
        void drawGui() override;

        void keyPressed(int key) override;
        void keyReleased(int key) override;
        void mouseMoved(int x, int y ) override;
        void mouseDragged(int x, int y, int button) override;
        void mousePressed(int x, int y, int button) override;
        void mouseReleased(int x, int y, int button) override;
        void mouseEntered(int x, int y) override;
        void mouseExited(int x, int y) override;
        void windowResized(int w, int h) override;
        void dragEvent(ofDragInfo dragInfo) override;
        void gotMessage(ofMessage msg) override;
		
		// Ball simulation
        void launchBall();
        glm::vec2 ballPos, ballVelocity;
        float ballSize = 10.f; // px
        float ballSpeed = 500.f; // px/second

#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
        virtual bool populateXmlSettings(pugi::xml_node& _node) override;
        virtual bool retrieveXmlSettings(pugi::xml_node& _node) override;
#endif

        // Timeline
#ifdef ofxSA_TIMELINE_ENABLE
        bool onTimelineRestart(std::size_t& _loopCount);
        bool onTimelineFrame(ofxSATimeCounters& _counters);
        bool onTimelinePause(bool& _paused);
        bool onTimelineSeek(ofxSATimeCounters& _counters);
#endif
};

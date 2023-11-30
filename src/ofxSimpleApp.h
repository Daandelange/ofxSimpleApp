#pragma once

#include "ofMain.h"
#include "ofxSimpleAppConfig.h"

#include "ofxImGui.h"
#include "ofxImGuiLoggerChannel.h"

#ifdef ofxSA_XML_ENGINE_PUGIXML
#	include "pugixml.hpp"
#else
#	include "ofxXmlSettings.h"
#endif

#ifdef ofxSA_SYPHON_OUTPUT
#include "ofxSyphon.h"
#endif

//enum AppState {
//    SETUP,
//    WAITING,
//    CAPTURING,
//    ENCODING,
//    ENCODE_WAIT,
//    HALTED,
//    OTHER
//};

class ofxSimpleApp : public ofBaseApp {

	public:
		ofxSimpleApp();
		~ofxSimpleApp() override;
		void setup() override;
		void exit() override;
		void update() override;
		void draw() override;
		virtual void drawGui();

		void renderGui();
		void toggleGui();
		void updateCursorForGui();
		//void drawLogger();

		void keyPressed(ofKeyEventArgs &e) override;
		void keyReleased(ofKeyEventArgs &e) override;
//		void mouseMoved(int x, int y );
//		void mouseDragged(int x, int y, int button);
//		void mousePressed(int x, int y, int button);
//		void mouseReleased(int x, int y, int button);
//		void mouseEntered(int x, int y);
//		void mouseExited(int x, int y);
//		void windowResized(int w, int h);
//		void dragEvent(ofDragInfo dragInfo);
//		void gotMessage(ofMessage msg);

		//--------------------------------------------------------------
		virtual void audioRequested(float * output, int bufferSize, int nChannels) override;

protected:
		// Main GUI
		ofxImGui::Gui gui;
		bool bShowGui = false;
		bool bShowAboutWindow = false;
		bool bDebugGlobal = false;
		bool bShowImGuiMetrics = false;
		bool bShowImGuiDebugWindow = false;
		float FPSHistory[ofxSA_FPS_HISTORY_SIZE];

		static ofxImGui::BaseTheme* imguiTheme;
		static bool bUseDarkTheme;
		void loadImGuiTheme();

		// Xml settings
#ifdef ofxSA_XML_ENGINE_PUGIXML
		//pugi::xml_document xml;
		virtual bool populateXmlSettings(pugi::xml_node& _node)=0;
		virtual bool retrieveXmlSettings(pugi::xml_node& _node)=0;
		bool ofxSA_populateXmlSettings(pugi::xml_node& _node);
		bool ofxSA_retrieveXmlSettings(pugi::xml_node& _node);
#else
		ofxXmlSettings xml; // Todo: Move away from this to use PugiXML (fast, multiplatform, great API). TinyXML is very slow.
		virtual bool populateXmlSettings() = 0;
		virtual bool retrieveXmlSettings() = 0;
#endif
		bool loadXmlSettings();
		bool saveXmlSettings();


		// Parameters


		// App State
//		AppState appState = AppState::SETUP;
//		//template<typename... Args>
//		void setAppState(const AppState& _appState);
//		//template <typename T>
//		template <typename... Args>
//		bool isAppState(const AppState& _appState, Args... args) const;
//		std::string getAppStateName(const AppState& _appState, std::string _notFoundString = "404" ) const;

		// Logging
		bool bShowLogs = bShowLogs = false;
		std::shared_ptr<ofxImGui::LoggerChannel> logChannel;
		static std::pair<ofLogLevel, std::string> ofLogLevels[];

#ifdef ofxSA_SYPHON_OUTPUT
		// Syphon
		ofxSyphonServer syphonServer;
		bool bEnableSyphonOutput = false;
		virtual void publishSyphonTexture();
#endif

};

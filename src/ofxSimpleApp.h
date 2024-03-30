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
#	include "ofxSyphon.h"
#endif

#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
#	include "ofxSimpleAppCanvas.h"
#endif

#ifdef ofxSA_TIMELINE_ENABLE
#include "ofxSimpleAppTimeline.h"
#endif

#ifdef ofxSA_TEXRECORDER_ENABLE
#include "ofxFFmpegRecorder.h"
#include "ofxFastFboReader.h"
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
//		void dragEvent(ofDragInfo dragInfo);
//		void gotMessage(ofMessage msg);
		void windowResized(int w, int h) override;

		//--------------------------------------------------------------
		virtual void audioRequested(float * output, int bufferSize, int nChannels) override;

protected:
		// Main GUI
		ofxImGui::Gui gui;
		bool bShowGui = false;
		bool bShowAboutWindow = false;
		bool bDebugGlobal = false;
		bool bShowImGuiMetrics = false;
		bool bShowImGuiDemo = false;
		bool bShowImGuiDebugWindow = false; // todo: rename bShowImGuiDebugLog
		float FPSHistory[ofxSA_FPS_HISTORY_SIZE];
		static const int curYear;

		static ofxImGui::BaseTheme* imguiTheme;
		static bool bUseDarkTheme;
		void loadImGuiTheme();
		void ImGuiDrawMenuBar();
		void ImGuiDrawAboutWindow();
#if ofxSA_UI_DOCKING_ENABLE_DOCKSPACES == 1
		void ImGuiDrawDockingSpace();
		ofRectangle dockingViewport;
		void onImguiViewportChange(){
			// Update canvas
			ofRectangle vp = getViewport();
			canvas.setScreenRect(vp.width, vp.height, vp.x, vp.y);
		}
#endif
		virtual void onViewportChange(){
			// Update canvas
			//ofRectangle vp = getViewport();
			//canvas.setScreenRect(vp.width, vp.height, vp.x, vp.y);
		}

		virtual void onContentResize(unsigned int _width, unsigned int _height){

		}
		//void updateViewport();
		ofRectangle getViewport() const;
		//ofRectangle curViewport;

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

		// Output canvas
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
	public:
		unsigned int getCanvasResolutionX() const;
		unsigned int getCanvasResolutiony() const;
		void onCanvasViewportResize(ofRectangle& args);
		void onCanvasContentResize(ContentResizeArgs& _args);
	protected:
		ofxSimpleAppCanvas canvas;
#endif

#ifdef ofxSA_TIMELINE_ENABLE
	protected:
#	if !ofxSA_TIMELINE_SINGLETON
		ofxSATimeline timeline;
#endif
		bool bShowTimeClockWindow = true;
		virtual void ImGuiDrawTimeline();
#endif

#ifdef ofxSA_TEXRECORDER_ENABLE
	public:
		bool startRecordingCanvas();
		bool stopRecordingCanvas();
	protected:
		ofxFFmpegRecorder m_Recorder;
		bool isRecordingCanvas = false;
		ofxFastFboReader fastFboReader = {3};
		void recordCanvasFrame();
		ofPixels recordedPixels;
#endif
};

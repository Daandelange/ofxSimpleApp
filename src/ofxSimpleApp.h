#pragma once

#include "ofMain.h"
#include "ofxSimpleAppConfig.h"

#include "ofxImGui.h"
#include "ofxImGuiLoggerChannel.h"

#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
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
#include "ofxPlayhead.h"
#endif

#ifdef ofxSA_TEXRECORDER_ENABLE
#include "ofxFFmpegRecorder.h"
#include "ofxFastFboReader.h"
#endif

#ifdef ofxSA_NDI_SENDER_ENABLE
#include "ofxNDI.h"
#endif

#if defined(ofxSA_SYPHON_OUTPUT) || defined(ofxSA_TEXRECORDER_ENABLE)
#include "ofFpsCounter.h"
#endif

#ifdef ofxSA_QUADWRAPPER_ENABLE
#	include "ofxGLWarper.h"
#endif

#ifdef TARGET_OSX
// Requires to install https://developer.nvidia.com/cuda-toolkit-archive
// Osx 10.12 : use Cuda toolkit 9.2.1
// Osx 10.13+ : use Cuda toolkit 10.2
//#include <cuda_runtime_api.h>
//#include <cuda.h>
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
		// note: authorises private member access to canvas
		template<class ofAppClass>
		friend int ofxSimpleAppGenericMain();

		ofxSimpleApp();
		~ofxSimpleApp() override;
		void setup() override;
		void exit() override;
		void update() override;
		void draw() override;
        virtual void drawScene();
		virtual void drawGui();

		void renderGui();
		void toggleGui();
		void updateCursorForGui();
		//void drawLogger();

		void keyPressed(ofKeyEventArgs &e) override;
		void keyReleased(ofKeyEventArgs &e) override;
//		void mouseMoved(int x, int y ) override;
//		void mouseDragged(int x, int y, int button) override;
//		void mousePressed(int x, int y, int button) override;
//		void mouseReleased(int x, int y, int button) override;
//		void mouseEntered(int x, int y) override;
//		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
//		void dragEvent(ofDragInfo dragInfo) override;
//		void gotMessage(ofMessage msg) override;

		//--------------------------------------------------------------
		virtual void audioRequested(float * output, int bufferSize, int nChannels) override;

protected:
        // Internals
        bool bExitOnNextUpdate = false;

		// Main GUI
		ofxImGui::Gui gui;
		bool bShowGui = false;
		bool bShowAboutWindow = false;
		bool bDebugGlobal = false;
		bool bShowImGuiMetrics = false;
		bool bShowImGuiDemo = false;
#ifdef OFXIMGUI_DEBUG
		bool bShowofxImGuiDebugWindow = false;
#endif
        bool bShowImGuiDebugLog = false;
		float FPSHistory[ofxSA_FPS_HISTORY_SIZE];
		static const int curYear;
		static ofxImGui::BaseTheme* imguiTheme;
        static unsigned int themeID;
        float imguiMenuHeight = -1;

		void loadImGuiTheme();
		void ImGuiDrawMenuBar();
		void ImGuiDrawAboutWindow();
#if ofxSA_UI_DOCKING_ENABLE_DOCKSPACES == 1
		void ImGuiDrawDockingSpace();
        ofRectangle dockingViewport; // Note: imgui coordinates (window/screen coords depending on ImGuiConfigFlags_ViewportsEnable)
        void onImguiViewportChange();
#endif
        virtual void onViewportChange();

        virtual void onContentResize(unsigned int _width, unsigned int _height);

		//void updateViewport();
		// Returns the document viewport
        ofRectangle getGuiViewport(bool returnScreenCoords=false) const;
        // Returns the document size
        ofRectangle getDocumentSize() const;
		//ofRectangle curViewport;

		// Xml settings
#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
		//pugi::xml_document xml;
        virtual bool populateXmlSettings(pugi::xml_node& _node){ return true; };
        virtual bool retrieveXmlSettings(pugi::xml_node& _node){ return true; };
		bool ofxSA_populateXmlSettings(pugi::xml_node& _node);
		bool ofxSA_retrieveXmlSettings(pugi::xml_node& _node);
#else
		ofxXmlSettings xml; // Todo: Move away from this to use PugiXML (fast, multiplatform, great API). TinyXML is very slow.
		virtual bool populateXmlSettings() = 0;
		virtual bool retrieveXmlSettings() = 0;
#endif
		bool loadXmlSettings(std::string _fileName = "");
		bool saveXmlSettings(std::string _fileName = "");
		const std::string savePath = ofxSA_XML_FOLDER;
		std::string saveName = ofxSA_XML_FILENAME;


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
		bool bShowLogs = false;
		std::shared_ptr<ofxImGui::LoggerChannel> logChannel;
		static std::pair<ofLogLevel, std::string> ofLogLevels[];

		// Frame flagger
#if ofxSA_NEWFRAME_FLAGGER == 1
		bool bNewFrame = true;
		inline void flagNewFrame(){ bNewFrame|=true; }; // to be called from drawScene() when new content has been drawn
		bool isNewFrame() const { return bNewFrame;}
#	if defined(ofxSA_SYPHON_OUTPUT) || defined(ofxSA_TEXRECORDER_ENABLE) || defined(ofxSA_NDI_SENDER_ENABLE)
		bool bRecordersOnlyPublishNewFrames = true;
#	endif
#endif

		// Syphon
#ifdef ofxSA_SYPHON_OUTPUT
		ofxSyphonServer syphonServer;
		bool bEnableSyphonOutput = false;
		ofFpsCounter syphonFps;
		virtual void publishSyphonTexture(); // todo: add force-re-publish arg ?
#endif

		// Output canvas
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
	public:
		unsigned int getCanvasResolutionX() const;
        unsigned int getCanvasResolutionY() const;
		void onCanvasViewportResize(ofRectangle& args);
		void onCanvasContentResize(ContentResizeArgs& _args);
	protected:
		ofxSimpleAppCanvas canvas;
#endif

#ifdef ofxSA_TIMELINE_ENABLE
	protected:
#	if !ofxSA_TIMELINE_SINGLETON
		ofxPlayhead timeline;
#	endif
		bool bShowTimeClockWindow = true;
		virtual void ImGuiDrawTimeline();
#endif

#ifdef ofxSA_TEXRECORDER_ENABLE
	public:
		virtual bool startRecordingCanvas();
		virtual bool stopRecordingCanvas();
		enum TexRecorderMode_ {
			TexRecorderMode_FFMPEG = 0,
			TexRecorderMode_PNG = 1,
		} texRecorderMode = TexRecorderMode_PNG;
	protected:
		ofxFFmpegRecorder m_Recorder;
		bool isRecordingCanvas = false;
		ofxFastFboReader fastFboReader = {1};
		virtual void recordCanvasFrame(); // You need to implement this once to send your frames, or call recordCanvasPixels() manually.
		void recordCanvasPixels(const ofPixels& _pixels);
		ofPixels recordedPixels;
		std::string recordingTargetName = ofxSA_TEXRECORDER_DEFAULT_FILENAME;
		std::string curRecordingName = "";
		bool bRecordAudioToo = false; // Unsupported yet ???
		std::string getNextRecordingName();
		static bool formatPngFilePath(std::string& _string, unsigned int _frame);
		bool bThreadedRecording = true;
		static void threadFnSavePng(const ofPixels& _pixels, std::string _filePath);
		const char* selectedCodec = nullptr;//ofxSA_TEXRECORDER_DEFAULT_CODEC;
		static const std::map<const char*, const char*> ffmpegRecordingFormats; // Pair of codec + extension
		unsigned int bitrateVideo = 12000;
		unsigned int bitrateAudio = 320;
		ofFpsCounter recorderFps;

#	ifdef ofxSA_TIMELINE_ENABLE
		int recordFrameRange[2] = {-1, -1};
		bool bRecorderStopOnLoop = true;

		bool onTimelineRestart(const std::size_t& _loopCount);
		bool onTimelineStop();
#	else
		float recordStartSeconds = -1.f;
		int recordStartFrame = -1.f;
		float recordLengthSeconds = 0.f;
#	endif
#endif

#ifdef ofxSA_NDI_SENDER_ENABLE
		ofxNDIsender ndiSender;
		bool startNdi();
		void stopNdi();
#endif

		// Quad wrapping
#ifdef ofxSA_QUADWRAPPER_ENABLE
		ofxGLWarper quadWarper;
		bool bEnableQuadWarper = false;
#endif

#ifdef ofxSA_BACKGROUND_CLEARING
	protected:
		bool bEnableClearing = true;
		ofFloatColor bgClearColor = ofxSA_BACKGROUND_CLEARING_COLOR;
#endif
#ifdef ofxSA_BACKGROUND_FADING
	protected:
		bool bEnableFading = true;
		ofFloatColor bgFadeColor = ofxSA_BACKGROUND_FADING_COLOR;
#endif
};

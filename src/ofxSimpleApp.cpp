#include <string>
#include "ofxSimpleApp.h"

#include "imgui.h"
#include "imgui_internal.h" // <-- advanced docking features from imgui internals...

// Gui Themes
#include "Spectrum.h"
#include "DarkTheme.h"
#include "DefaultTheme.h"
#ifdef ofxSA_GUI_THEME_CUSTOM_FILE
#	include ofxSA_GUI_THEME_CUSTOM_FILE
#endif

// Modules
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
#include "ofxTimeMeasurements.h"
#endif

#ifdef ofxSA_NDI_SENDER_ENABLE
#include "ofxNDIImGuiEx.h"
#endif

#include "ofxSimpleAppUtils.h"
//#include "ofGraphics.h" // for GL info stuff ?

#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
#	include "ofxPugiXMLHelpers.h"
#endif

#include "GLFW/glfw3.h" // for the runtime tab
//------
// STATICS
std::pair<ofLogLevel, std::string> ofxSimpleApp::ofLogLevels[] = {
	{ OF_LOG_VERBOSE,       ofGetLogLevelName(OF_LOG_VERBOSE    ) },
	{ OF_LOG_NOTICE,        ofGetLogLevelName(OF_LOG_NOTICE     ) },
	{ OF_LOG_WARNING,       ofGetLogLevelName(OF_LOG_WARNING    ) },
	{ OF_LOG_ERROR,         ofGetLogLevelName(OF_LOG_ERROR      ) },
	{ OF_LOG_FATAL_ERROR,   ofGetLogLevelName(OF_LOG_FATAL_ERROR) },
	{ OF_LOG_SILENT,        ofGetLogLevelName(OF_LOG_SILENT     ) }
};

unsigned int ofxSimpleApp::themeID = ofxSA_GUI_THEME_DEFAULT;
ofxImGui::BaseTheme* ofxSimpleApp::imguiTheme = nullptr;
const int ofxSimpleApp:: curYear = std::stoi(ofGetTimestampString("%Y").c_str());

//--------------------------------------------------------------
ofxSimpleApp::ofxSimpleApp()
    #ifdef ofxSA_TEXRECORDER_ENABLE
    : selectedCodec(ffmpegRecordingFormats.begin()->first)
    #endif
{
	// LOGGER SETUP
	logChannel = std::shared_ptr<ofxImGui::LoggerChannel>( new ofxImGui::LoggerChannel() );
	ofSetLoggerChannel(logChannel);

#ifdef ofxSA_DEBUG
	ofSetLogLevel(OF_LOG_VERBOSE);
#endif

	// Print where we are ...
	ofLogNotice("ofxSimpleApp") << "Running from working directory: " << ofFilePath::getCurrentWorkingDirectory();
}

//--------------------------------------------------------------
ofxSimpleApp::~ofxSimpleApp(){
	if(imguiTheme!=nullptr) delete imguiTheme;
}

//--------------------------------------------------------------
void ofxSimpleApp::setup(){

	// Now we have a GL context, give some GL info
#if defined( ofxSA_DEBUG )
	char *version = NULL;
	char *vendor = NULL;
	char *renderer = NULL;
	version =  (char*)glGetString(GL_VERSION);
	vendor =   (char*)glGetString(GL_VENDOR);
	renderer = (char*)glGetString(GL_RENDERER);
	ofLogVerbose(__FUNCTION__) << "ofxSimpleApp : GL information : " << version << " // " << vendor << " // " << renderer << "." ;
#endif

	// - - - - - - - -
	// OF environment
	ofSetEscapeQuitsApp(false);
	ofSetVerticalSync(true);
#if defined(ofxSA_FPS_CAP) && ofxSA_FPS_CAP != 0
	ofSetFrameRate(ofxSA_FPS_CAP);
#endif

#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
	//specify where the widget is to be drawn
	TIME_SAMPLE_SET_DRAW_LOCATION( TIME_MEASUREMENTS_BOTTOM_RIGHT );
	//TIME_SAMPLE_GET_INSTANCE()->removeEventHooks(); // drm draw ??
	//TIME_SAMPLE_GET_INSTANCE()->setAutoDraw(false); // disable auto draw !


	//TIME_SAMPLE_SET_AVERAGE_RATE(0.1);	//averaging samples, (0..1],
										//1.0 gets you no averaging at all
										//use lower values to get steadier readings
	TIME_SAMPLE_DISABLE_AVERAGE();	//disable averaging
	TIME_SAMPLE_SET_REMOVE_EXPIRED_THREADS(true); //inactive threads will be dropped from the table
#if defined(ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY) && ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY != 0
    TIME_SAMPLE_GET_INSTANCE()->setGlobalEnableDisableKey(ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY);
#endif

	//customize color
	//TIME_SAMPLE_GET_INSTANCE()->setHighlightColor(ofColor::yellow);

	//TIME_SAMPLE_GET_INSTANCE()->drawUiWithFontStash("fonts/UbuntuMono-R.ttf");
	//TIME_SAMPLE_GET_INSTANCE()->drawUiWithFontStash2("fonts/UbuntuMono-R.ttf");
    TIME_SAMPLE_GET_INSTANCE()->setEnabled(ofxSA_TIME_MEASUREMENTS_AUTO_ENABLED);

#endif

	// Window Title
	// Done in main.cpp, useless here
	//ofSetWindowTitle(ofxSA_APP_NAME);

	// - - - - - -
	// GUI SETUP

	// Gui Instance
    gui.setup(nullptr, false, ofxSA_UI_IMGUI_CONFIGFLAGS, ofxSA_UI_RESTORE_STATE);
    loadImGuiTheme();
    imguiMenuHeight = ImGui::GetFrameHeight(); // Initial value, will be overwritten when menu is drawn

#ifdef ofxSA_GUI_DEFAULT_HIDDEN
    bShowGui = !ofxSA_GUI_DEFAULT_HIDDEN;
#endif
	updateCursorForGui();

#ifdef ofxSA_SYPHON_OUTPUT
	// Syphon setup
	syphonServer.setName(ofxSA_APP_NAME);
#endif

	// Output FBO / Canvas setup
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
	canvas.setCanvasSize(ofxSA_CANVAS_OUTPUT_DEFAULT_WIDTH, ofxSA_CANVAS_OUTPUT_DEFAULT_HEIGHT);
    canvas.setViewportRect(); // todo: make this use viewports ?
	ofAddListener(canvas.onViewportResize, this, &ofxSimpleApp::onCanvasViewportResize);
	ofAddListener(canvas.onContentResize, this, &ofxSimpleApp::onCanvasContentResize);
#endif

#ifdef ofxSA_TIMELINE_ENABLE
    ofxSA_TIMELINE_GET(timeline).setFps(ofxSA_FPS_CAP);

#	if ofxSA_TIMELINE_AUTOSTART == true
	ofxSA_TIMELINE_GET(timeline).start();
#	endif

#   ifdef ofxSA_TEXRECORDER_ENABLE
    ofAddListener(ofxSA_TIMELINE_GET(timeline).onStart, this, &ofxSimpleApp::onTimelineRestart);
    ofAddListener(ofxSA_TIMELINE_GET(timeline).onStop, this, &ofxSimpleApp::onTimelineStop);
#   endif
#endif

#ifdef ofxSA_TEXRECORDER_ENABLE
    // Setup texture recorder
#   ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    m_Recorder.setup(true, false, glm::vec2(canvas.getCanvasWidth(), canvas.getCanvasHeight()) );
#   else
    m_Recorder.setup(true, false, glm::vec2(ofGetWidth(), ofGetHeight()) );
#   endif
    m_Recorder.setAudioConfig(1024,44100); // todo: make optional
    m_Recorder.setOverWrite(true); // Allow overwriting recording file destintion

    // Locate ffmpeg binary
    #if defined(TARGET_OSX)
    m_Recorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg"));
    #elif defined(TARGET_WIN32)
    m_Recorder.setFFmpegPath(ofToDataPath("ffmpeg/win/ffmpeg.exe"));
    //m_Recorder.setFFmpegPath(ofToDataPath("C:/dev/msys64/ThirdParty/ffmpeg/ffmpeg_build/bin/ffmpeg.exe", true));
    #endif

    // Set default codec
    m_Recorder.setVideoCodec(ofxSA_TEXRECORDER_DEFAULT_CODEC);
    
    // Configure FBO reader
    //fastFboReader.setAsync(false);
#endif

    // NDI
#ifdef ofxSA_NDI_SENDER_ENABLE
    ndiSender.SetReadback(false);
    ndiSender.SetAsync(false);

#   ifdef ofxSA_TIMELINE_ENABLE
    ndiSender.SetFrameRate((int)ofxSA_TIMELINE_GET(timeline).getFps());
#   else
    ndiSender.SetFrameRate(ofGetTargetFrameRate());
#   endif

    startNdi();
#endif

	// Quad wrapping
#ifdef ofxSA_QUADWRAPPER_ENABLE
	auto docRect = getDocumentSize();
	quadWarper.setup(0, 0, docRect.getWidth(), docRect.getHeight()); // initial size = fulscreen
	quadWarper.load();// try autoload from save
	quadWarper.drawSettings.bDrawRectangle = true;
	quadWarper.drawSettings.bDrawCorners = true;
	bEnableQuadWarper = false;
#endif

	// Restore settings on launch
	loadXmlSettings();

    // This one is not automagically bound by OF (seems only supported in GLFW windows)
    //ofAddListener(ofGetCurrentWindow()->events().windowMoved, this, &ofxSimpleApp::windowMoved, OF_EVENT_ORDER_APP);
}

void ofxSimpleApp::exit(){
#ifdef ofxSA_TEXRECORDER_ENABLE
	// Ensure encoding the current recording correctly
	if(isRecordingCanvas){
		stopRecordingCanvas();
	}

	// Remove listeners
#   ifdef ofxSA_TIMELINE_ENABLE
#       ifdef ofxSA_TEXRECORDER_ENABLE
    ofRemoveListener(ofxSA_TIMELINE_GET(timeline).onStart, this, &ofxSimpleApp::onTimelineRestart);
    ofRemoveListener(ofxSA_TIMELINE_GET(timeline).onStop, this, &ofxSimpleApp::onTimelineStop);
#       endif
#   endif
#endif

#ifdef ofxSA_NDI_SENDER_ENABLE
    ndiSender.ReleaseSender();
#endif

    // This one is not automagically bound by OF (seems only supported in GLFW windows)
    //ofRemoveListener(ofGetCurrentWindow()->events().windowMoved, this, &ofxSimpleApp::windowMoved, OF_EVENT_ORDER_APP);
}

//--------------------------------------------------------------
void ofxSimpleApp::update(){
    // Exit ?
    if(bExitOnNextUpdate){
        ofExit();
        bExitOnNextUpdate = false;
    }

    // FPS plotting
    // Todo: Use a more precise measure ? Measure frame time too ?
    syncHistogram<float, double>(FPSHistory, ofGetFrameRate());

#ifdef ofxSA_TIMELINE_ENABLE
    ofxSA_TIMELINE_GET(timeline).tickUpdate();
#endif

    // FPS trackers
#ifdef ofxSA_TEXRECORDER_ENABLE
    recorderFps.update();
#endif

#ifdef ofxSA_SYPHON_OUTPUT
    syphonFps.update();
#endif
}

//--------------------------------------------------------------
void ofxSimpleApp::draw(){
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TS_SCOPE("ofxSimpleApp::draw()");
#endif
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TSGL_START("ofxSimpleApp::draw()");
#endif

    // Timeline ticking
#ifdef ofxSA_TIMELINE_ENABLE
    ofxSA_TIMELINE_GET(timeline).tickFrame();
#endif

    // Call custom user draw function before drawing the texture
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TS_START("ofxSimpleApp::drawScene()");
#endif

    // Reset new frame
#if ofxSA_NEWFRAME_FLAGGER == 1
    bNewFrame = false;
#endif

    // Start canvas ?
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    canvas.fbo.begin();
#endif
    // BG erasure
#ifdef ofxSA_BACKGROUND_CLEARING
    if(bEnableClearing){
        ofClear(bgClearColor);
    }
//#   endif
#endif
    // Bg Fading (subtract drawn pixels from current pixels)
#ifdef ofxSA_BACKGROUND_FADING
    if(bEnableFading){
        // Enable blendmode + style
        ofPushStyle();
        ofFill();
#ifdef ofxSA_TIMELINE_ENABLE
        double tFPS = ofxSA_TIMELINE_GET(timeline).getFps();
        double delta = ofxSA_TIMELINE_GET(timeline).getCounters().tDelta;
#else
        double tFPS = ofGetTargetFrameRate();
        double delta = 1.0/ofGetFrameRate();
#endif
        ofSetColor(bgFadeColor*tFPS*delta);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE);
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);

        ofDrawRectangle(getDocumentSize());

        // Restore blendmode + style
        glDisable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
        ofPopStyle();

#if ofxSA_NEWFRAME_FLAGGER == 1
        flagNewFrame();
#endif
    }
#endif

#ifdef ofxSA_QUADWRAPPER_ENABLE
    if(bEnableQuadWarper){
        // Erase BG as wrapped tex is not guaranteed to cover the whole screen
        ofClear(0,0,0,255);
        // activate wrapper
        quadWarper.begin();

    }
#else
//    std::cout << "Holy ****!" << std::endl;
#endif

    // Call the custom draw function
    drawScene();

#ifdef ofxSA_QUADWRAPPER_ENABLE
    if(bEnableQuadWarper) quadWarper.end();
#endif

#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TS_STOP("ofxSimpleApp::drawScene()");
#endif
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    canvas.fbo.end();
#endif

    // Draw canvas texture to viewport
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    canvas.draw();
#endif

    // Export the image before the GUI
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TS_START("Outputs and recording");
#endif
    // Syphon
#ifdef ofxSA_SYPHON_OUTPUT
    publishSyphonTexture();
#endif
    // TexRecorder
#ifdef ofxSA_TEXRECORDER_ENABLE
    recordCanvasFrame();
#endif
    // NDI
#ifdef ofxSA_NDI_SENDER_ENABLE
#   ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    ndiSender.SendImage(canvas.fbo);
#   else
    ofImage pix;
    auto w = ofGetWindowWidth();
    auto h = ofGetWindowHeight();
    pix.setUseTexture(true);
    pix.grabScreen(0, 0, w, h);
    ndiSender.SendImage(pix);
#   endif
#endif
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TS_STOP("Outputs and recording");
#endif

    // Gui Rendering
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TS_START("Render Gui");
#endif
    renderGui();
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TS_STOP("Render Gui");
#endif
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
    TSGL_STOP("ofxSimpleApp::draw()");
#endif
}

//--------------------------------------------------------------
void ofxSimpleApp::drawScene(){
#if ofxSA_NEWFRAME_FLAGGER == 1
        flagNewFrame();
#endif
}

//--------------------------------------------------------------
#ifdef ofxSA_SYPHON_OUTPUT
void ofxSimpleApp::publishSyphonTexture(){
    // Limit emissions only to new frames
#if ofxSA_NEWFRAME_FLAGGER == 1
    if(bRecordersOnlyPublishNewFrames && !isNewFrame()) return;
#endif

    // Disabled ?
    if(!bEnableSyphonOutput) return;

#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    syphonServer.publishTexture(&canvas.fbo.getTexture());
#else
        // By default, this publishes the full window.
        // You can override this to publish a custom texture.
        syphonServer.publishScreen();
#endif
    syphonFps.newFrame();
}
#endif

//--------------------------------------------------------------
#ifdef ofxSA_TEXRECORDER_ENABLE

std::string glFormatToFFmpeg(int glFormat) {
    // (Untested formats are commented)
    static const std::map<int, std::string> formatMap = {
        // Common formats
        {GL_ALPHA, "a8"},
        {GL_RGB, "rgb24"},
        {GL_RGBA, "rgba"},
        {GL_BGR, "bgr24"},
        {GL_BGRA, "bgra"},
//        {GL_RED, "r8"},
//        {GL_GREEN, "g8"},
//        {GL_BLUE, "b8"},
//        {GL_RG, "rg8"},
        {GL_LUMINANCE, "gray"},
//        {GL_LUMINANCE_ALPHA, "ya8"},

        // 32-bit integer mappings
//        {GL_RGBA_INTEGER, "rgba32"},
//        {GL_BGRA_INTEGER, "bgra32"},
//        {GL_RGB_INTEGER, "rgb32"},
//        {GL_BGR_INTEGER, "bgr32"},
//        {GL_RED_INTEGER, "r32"},
//        {GL_GREEN_INTEGER, "g32"},
//        {GL_BLUE_INTEGER, "b32"},
//        {GL_RG_INTEGER, "rg32"},

        // Float mappings
//        {GL_R32F, "gray32f"},
//        {GL_RG32F, "ya32f"},
//        {GL_RGB32F, "rgb32f"},
//        {GL_RGBA32F, "rgba32f"},
    };

    auto it = formatMap.find(glFormat);
    if (it != formatMap.end()) {
        return it->second;
    } else {
        return "";//"unsupported_format_mapping";
    }
}

std::string ofxSimpleApp::getNextRecordingName() {
    // Set new target file if empty
    if(recordingTargetName.length()<1){
        recordingTargetName = ofxSA_TEXRECORDER_DEFAULT_FILENAME;
    }

    bool bCodecFound = selectedCodec && (ffmpegRecordingFormats.find(selectedCodec)!=ffmpegRecordingFormats.end());
    const char* extension = (texRecorderMode == TexRecorderMode_PNG) ? "/frame-000000.png" : bCodecFound?ffmpegRecordingFormats.at(selectedCodec):ofxSA_TEXRECORDER_DEFAULT_CODEC;

    // Grab new unique filename from requested name
    return getNewFileName(ofxSA_TEXRECORDER_DEFAULT_OUTPUT_FOLDER, recordingTargetName.c_str(), extension, true, "_");
}

bool ofxSimpleApp::formatPngFilePath(std::string& _string, unsigned int _frame){
    std::size_t pos = _string.find_last_of("000000.png"); // Note: returns end of found string
    if(pos != std::string::npos){
        snprintf(&_string[pos-6u-3u], 6u+4u+1u, "%06u.png", _frame);
        return true;
    }
    return false;
}

bool ofxSimpleApp::startRecordingCanvas(){
    // Checks
#   ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    if(!canvas.fbo.getTexture().isAllocated()){
        ofLogNotice("ofxSimpleApp::startRecordingCanvas()") << "Can't start recording, the canvas is not allocated !";
        return false;
    }
#   endif

    // Sync timeline
#ifdef ofxSA_TIMELINE_ENABLE
	ofxPlayhead& tl = ofxSA_TIMELINE_GET(timeline);
	tl.startNextFrame(glm::max(0,recordFrameRange[0]));
#   else
    recordStartSeconds = ofGetElapsedTimef();
    recordStartFrame = ofGetFrameNum()+1; // +1 because recording starts next frame. Important.
#endif

    // Set new target file
    curRecordingName = ofxSA_TEXRECORDER_DEFAULT_OUTPUT_FOLDER;
    curRecordingName += "/";
    curRecordingName += getNextRecordingName();

    if(texRecorderMode == TexRecorderMode_FFMPEG){
        m_Recorder.setOutputPath(ofToDataPath(curRecordingName));

        // Recording settings
        m_Recorder.setVideoCodec(selectedCodec);
#   ifdef ofxSA_CANVAS_OUTPUT_ENABLE
        int srcPxFormat = canvas.fbo.getTexture().getTextureData().glInternalFormat;
        auto ffmpegFormat = glFormatToFFmpeg(srcPxFormat);
#   else
        auto ffmpegFormat = glFormatToFFmpeg(GL_BGRA);
#   endif

        if(ffmpegFormat.length()<1)
            ofLogWarning("ofxSimpleApp::startRecordingCanvas()") << "The canvas has an unsupported pixel format, you have to map your GL_format to the ffmpeg pixel format !";
        m_Recorder.setInputPixelFormat(ffmpegFormat);

        // Timings
#ifdef ofxSA_TIMELINE_ENABLE
        const unsigned int fromRec = recordFrameRange[0]>0 ? recordFrameRange[0] : 0;
        const unsigned int toRec = recordFrameRange[1]>0 ? recordFrameRange[1] : tl.getTotalFrames();
        float duration = float(toRec-fromRec)*(1.f/tl.getFps());
        //tl.goToFrame(fromRec, false, false);

        m_Recorder.setCaptureDuration(duration);
        m_Recorder.setFps(tl.getFps());
#else
        // 0 = infinite too :)
        m_Recorder.setCaptureDuration(recordLengthSeconds);
        m_Recorder.setFps(ofGetTargetFrameRate());
#endif

        // Audio
        m_Recorder.setBitRate(12000); // Dodo: parameterize this
        m_Recorder.setRecordAudio(bRecordAudioToo); // todo: make optional

        // Dimensions
#   ifdef ofxSA_CANVAS_OUTPUT_ENABLE
        m_Recorder.setWidth(canvas.getCanvasWidth());
        m_Recorder.setHeight(canvas.getCanvasHeight());
#   else
        m_Recorder.setWidth(ofGetWidth());
        m_Recorder.setHeight(ofGetHeight());
#   endif

        ofLogNotice("ofxSimpleApp::startRecordingCanvas()")
                << "Start recording FFMPEG to destination = " << curRecordingName
#ifdef ofxSA_TIMELINE_ENABLE
                << "From=" << fromRec*(1.f/tl.getFps()) << "s, To="<< toRec*(1.f/tl.getFps()) << "s, Duration="<< duration << "s."
#else
                << "From=" << ofGetFrameNum() << "f = " << ofGetElapsedTimef() << "s, Duration="<< recordLengthSeconds << "s."
#endif
                << std::endl;

        isRecordingCanvas = m_Recorder.startCustomRecord();
    }
    else if(texRecorderMode == TexRecorderMode_PNG){
        // Create folder if it doesn't exist yet
        ofDirectory dir = ofDirectory(ofFilePath::getEnclosingDirectory(curRecordingName, true));
        if(!dir.exists()){
            dir.create(false);
        }
        ofLogNotice("ofxSimpleApp::startRecordingCanvas()") << "Start recording PNG to destination = " << dir.path() << std::endl;

            isRecordingCanvas = true;
    }

    return isRecordingCanvas;
}
bool ofxSimpleApp::stopRecordingCanvas(){
    if(texRecorderMode == TexRecorderMode_FFMPEG){
        m_Recorder.stop();
    }
    else if(texRecorderMode == TexRecorderMode_PNG){
        // Nothing to stop
    }
    isRecordingCanvas = false;

    // Pause timeline playback on record end
    // Todo: Make this optional !
#ifdef ofxSA_TIMELINE_ENABLE
    ofxSA_TIMELINE_GET(timeline).pause();
#endif

    return isRecordingCanvas == false;
}

void ofxSimpleApp::recordCanvasFrame(){

#if ofxSA_NEWFRAME_FLAGGER == 1
    if(bRecordersOnlyPublishNewFrames && !isNewFrame()) return;
#endif

    if (isRecordingCanvas ) {
        bool bDataLoaded = false;
#   ifndef ofxSA_CANVAS_OUTPUT_ENABLE
        // No FBO --> save window viewport
        auto renderer = ofGetGLRenderer();
        if(renderer != nullptr){
            renderer->saveFullViewport(recordedPixels);
            bDataLoaded = true;
        }
#   else
        // Read FBO to pixels
#       ifdef ofxSA_TEXRECORDER_USE_OFXFASTFBOREADER
        fastFboReader.setAsync(false);
        int glFormat = canvas.fbo.getTexture().texData.glInternalFormat;
        bDataLoaded = fastFboReader.readToPixels(canvas.fbo, recordedPixels, ofGetImageTypeFromGLType(glFormat));
#       else
        //canvas.fbo.updateTexture(0);
        canvas.fbo.readToPixels(recordedPixels); // slow OF way
        bDataLoaded = true;
#       endif
#   endif

        // Got a frame and valid data ?
        if( bDataLoaded && (recordedPixels.getWidth() > 0 && recordedPixels.getHeight() > 0) ){
            recordCanvasPixels(recordedPixels);
        }
        // Error grabbing frame data
        else {
            // Todo: ofLogError....
            std::cout << "failed readToPixels !" << std::endl;
        }
	}
}
#include <future> // for async
void ofxSimpleApp::recordCanvasPixels(const ofPixels& _pixels){
#if ofxSA_NEWFRAME_FLAGGER == 1
    if(bRecordersOnlyPublishNewFrames && !isNewFrame()) return;
#endif

    if( isRecordingCanvas && _pixels.getWidth() > 0 && _pixels.getHeight() > 0){
#ifdef ofxSA_TIMELINE_ENABLE
        const auto& tl = ofxSA_TIMELINE_GET(timeline);
        unsigned int curFrame = tl.getFrameNum();
        bool realTimeRecording = tl.getPlayMode()!=ofxPlayheadMode_Offline;

        // Check autoStop (is this the right place to do this??)
        const bool autoStop = (recordFrameRange[1] > 0 && tl.getFrameNum() > recordFrameRange[1]+1);
#else
        unsigned int curFrame = ofGetFrameNum()-recordStartFrame;
        bool realTimeRecording = true;
        const bool autoStop = (recordStartSeconds >= 0) && recordLengthSeconds>0 && (ofGetElapsedTimef() > recordStartSeconds+recordLengthSeconds);
#endif

		// Route the pixels to the selected recorder...
		if (texRecorderMode == TexRecorderMode_FFMPEG){
			if(m_Recorder.isRecording()) {
				if(!m_Recorder.addFrame(_pixels, realTimeRecording)){
					ofLogError("ofxSimpleApp::recordCanvasPixels()") << "FFMpeg refused to add the frame ("<< curFrame << ") data to the sequence.";
				}
			}
		}
		else if(texRecorderMode == TexRecorderMode_PNG){
			std::string frameFile = curRecordingName;
			if(formatPngFilePath(frameFile, curFrame)){
				std::string path = ofToDataPath(frameFile, true);
				if(bThreadedRecording){
					// Todo: probably better to use 1 single dedicated thread ?
					auto result = std::async(std::launch::async, ofxSimpleApp::threadFnSavePng, _pixels, path);
				}
				else {
					if(!ofSaveImage(_pixels, path)){
						ofLogError("ofxSimpleApp::recordCanvasPixels()") << "Couldn't save frame " << curFrame << " as a PNG !";
					}
				}
			}
			else {
				ofLogError("ofxSimpleApp::recordCanvasPixels()") << "Couldn't format the PNG file path for frame " << curFrame << ".";
			}
		}

		if(autoStop){
			ofLogNotice("ofxSimpleApp::recordCanvasFrame()") << "Auto-stopping current recording on "
#ifdef ofxSA_TIMELINE_ENABLE
				<< "frame=" << tl.getFrameNum();//recordFrameRange[1];
#else
				<< "elapsedSeconds=" << (ofGetElapsedTimef() - recordStartSeconds) << " (target=" << recordLengthSeconds << ")";
#endif
			stopRecordingCanvas();
			return;
		}
	}
}

void ofxSimpleApp::threadFnSavePng(const ofPixels& _pixels, std::string _filePath){
	if(!ofSaveImage(_pixels, _filePath)){
		ofLogError("ofxSimpleApp::recordCanvasPixels()") << "Couldn't save file `"<< _filePath <<"` as a PNG !";
	}
}

// These combinations have only been tested on OSX...
const std::map<const char*, const char*> ofxSimpleApp::ffmpegRecordingFormats = {
	{"libx264", ".mp4"},
	{"hap", DEFAULT_TEXTURE_RECORDER_CONTAINER_EXT},
	{"hevc", DEFAULT_TEXTURE_RECORDER_CONTAINER_EXT},
	{"mpeg4", ".mp4"},
	{"mjpeg", DEFAULT_TEXTURE_RECORDER_CONTAINER_EXT},
	{"jpeg2000", DEFAULT_TEXTURE_RECORDER_CONTAINER_EXT},
	{"webm", ".webm"},
};

#endif // end TEXRECORDER_ENABLE

//--------------------------------------------------------------
void ofxSimpleApp::drawGui(){
    // Draw GUI
    if(bShowGui){

        // Menu BAR
        ImGuiDrawMenuBar();

        // Docking
#if ofxSA_UI_DOCKING_ENABLE_DOCKSPACES == 1
        ImGuiDrawDockingSpace();
#endif

#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
        // The canvas HUD
        canvas.drawGuiViewportHUD();
#endif

#ifdef ofxSA_TIMELINE_ENABLE
        ImGuiDrawTimeline();
#endif

        // About
        ImGuiDrawAboutWindow();

        if(bShowLogs && logChannel) ImGuiEx::DrawLoggerChannel("Logger", logChannel, &bShowLogs);
    }
}

// ---
void ofxSimpleApp::renderGui(){
    if(bShowGui){
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
        TS_SCOPE("GUI Compositing");
#endif
        gui.begin();

        drawGui();

        // Force metrics at the end of ImGui submissions
        if(bShowImGuiDemo)          ImGui::ShowDemoWindow(&bShowImGuiDemo);
        if(bShowImGuiDebugLog)   ImGui::ShowDebugLogWindow(&bShowImGuiDebugLog);
        if(bShowImGuiMetrics)       ImGui::ShowMetricsWindow(&bShowImGuiMetrics);

        gui.end();

        gui.draw();
    }
}

void ofxSimpleApp::toggleGui(){
    bShowGui = !bShowGui;
    updateCursorForGui();
    onImguiViewportChange(); // Hiding menu / gui changes viewport
}

// Todo : Restore cursor according to user setting ? (some apps might wanna use a cursor ?)
void ofxSimpleApp::updateCursorForGui(){
    if(bShowGui){
        ofShowCursor();
    }
    else {
        ofHideCursor();
    }
}

//--------------------------------------------------------------
void ofxSimpleApp::keyPressed(ofKeyEventArgs &e){
    // Don't interfere with ImGui
    auto& io = ImGui::GetIO();
    if(io.WantCaptureKeyboard) return;

    bool doBubble = false;
#if defined(ofxSA_TIMELINE_PAUSE_KEY) && ofxSA_TIMELINE_PAUSE_KEY != 0
    if(e.key == 32){ // 32 = SPACE
        ofxSA_TIMELINE_GET(timeline).togglePause();
    }
    else
#endif

    // Check keyboard shortcuts
    if(e.hasModifier(MOD_KEY)){
        // Fullscreen
        if(e.keycode == 70){ // 70 = F
            ofToggleFullscreen();
        }
        // Show GUI
        else if(e.keycode == 71){ // 72 = G (H already taken in osx)
            toggleGui();
        }
        // Exit ?
        else if(e.keycode == 81){ // 81 = Q
            ofExit();
        }
        // Save ?
        else if(e.keycode == 83){ // 83 = S
            saveXmlSettings();
        }
        // Load ?
        else if(e.keycode == 76){ // 76 = L
            loadXmlSettings();
        }
        else doBubble = true;
    }
    else doBubble = true;

    // Call parenting function, which eventually calls ofApp::keyPressed(int key) for users who implemented it
    if(doBubble) ofBaseApp::keyPressed(e);
}

////--------------------------------------------------------------
void ofxSimpleApp::keyReleased(ofKeyEventArgs &e){
    // Don't interfere with ImGui
    auto& io = ImGui::GetIO();
    if(io.WantCaptureKeyboard) return;

    // Call parenting function, which eventually calls ofApp::keyPressed(int key) for users who implemented it
    ofBaseApp::keyPressed(e);
}

////--------------------------------------------------------------
//void ofxSimpleApp::mouseMoved(int x, int y ){

//}

////--------------------------------------------------------------
//void ofxSimpleApp::mouseDragged(int x, int y, int button){

//}

////--------------------------------------------------------------
//void ofxSimpleApp::mousePressed(int x, int y, int button){

//}

////--------------------------------------------------------------
//void ofxSimpleApp::mouseReleased(int x, int y, int button){

//}

////--------------------------------------------------------------
//void ofxSimpleApp::mouseEntered(int x, int y){

//}

////--------------------------------------------------------------
//void ofxSimpleApp::mouseExited(int x, int y){

//}

//--------------------------------------------------------------
void ofxSimpleApp::windowResized(int w, int h){
    onImguiViewportChange();
}

////--------------------------------------------------------------
//void ofxSimpleApp::gotMessage(ofMessage msg){

//}

////--------------------------------------------------------------
//void ofxSimpleApp::dragEvent(ofDragInfo dragInfo){

//}

// Dummy declaration
void ofxSimpleApp::audioRequested(float * output, int bufferSize, int nChannels){}


void ofxSimpleApp::loadImGuiTheme(){
	ofxImGui::BaseTheme* theme = nullptr;
	switch(themeID){
		case ofxSA_GUI_THEME_DARK:
			theme = (ofxImGui::BaseTheme*) new DarkTheme();
			break;
		case ofxSA_GUI_THEME_LIGHT:
			theme = (ofxImGui::BaseTheme*) new Spectrum();
			break;
		case ofxSA_GUI_THEME_IMGUI_DEFAULT:
			theme = (ofxImGui::BaseTheme*) new ofxImGui::DefaultTheme();
			break;
#ifdef ofxSA_GUI_THEME_CUSTOM_FILE
        case ofxSA_GUI_THEME_CUSTOM:
            theme = (ofxImGui::BaseTheme*) new ofxSA_GUI_THEME_CUSTOM_CLASS();
            break;
#endif
        default:
            theme = (ofxImGui::BaseTheme*)new DarkTheme();
            break;
    }
    gui.setTheme(theme);
}

void ofxSimpleApp::ImGuiDrawMenuBar(){
    if( ImGui::BeginMainMenuBar() ){
        if(ImGui::BeginMenu( ofxSA_APP_NAME )){
            // Info
            if(ofxSA_APP_COPYRIGHT_START_YEAR != 0 && ofxSA_APP_COPYRIGHT_START_YEAR != curYear){
                ImGui::TextDisabled("Copyright %i-%i %s", ofxSA_APP_COPYRIGHT_START_YEAR, curYear, ofxSA_APP_AUTHOR);
            }
            else {
                ImGui::TextDisabled("Copyright %i %s", curYear, ofxSA_APP_AUTHOR);
            }
            ImGui::TextDisabled("Version %d.%d.%d", ofxSA_APP_VERSION_MAJOR, ofxSA_APP_VERSION_MINOR, ofxSA_APP_VERSION_PATCH );
            ImGui::Dummy({ofxSA_UI_MARGIN, ofxSA_UI_MARGIN});
            ImGui::Separator();

            // About
            if(ImGui::MenuItem("About")){
                bShowAboutWindow = !bShowAboutWindow;
            }

            // Preferences
            if(ImGui::BeginMenu("Preferences")){
                // Logging
                ImGui::SeparatorText("Logging");
                ofLogLevel curLogLevel = ofGetLogLevel();
                static std::pair<ofLogLevel, std::string> ofLogLevels[] = {
                    { OF_LOG_VERBOSE,       ofGetLogLevelName(OF_LOG_VERBOSE    ) },
                    { OF_LOG_NOTICE,        ofGetLogLevelName(OF_LOG_NOTICE     ) },
                    { OF_LOG_WARNING,       ofGetLogLevelName(OF_LOG_WARNING    ) },
                    { OF_LOG_ERROR,         ofGetLogLevelName(OF_LOG_ERROR      ) },
                    { OF_LOG_FATAL_ERROR,   ofGetLogLevelName(OF_LOG_FATAL_ERROR) },
                    { OF_LOG_SILENT,        ofGetLogLevelName(OF_LOG_SILENT     ) }
                };
                ImGui::SetNextItemWidth(80); // Don't span all width by default
                if( ImGui::BeginCombo("##Log level", ofGetLogLevelName(curLogLevel).c_str()) ){
                    for(const auto& logLevel : ofLogLevels){
                        if(ImGui::Selectable(logLevel.second.c_str(), curLogLevel == logLevel.first ) ){
                            ofSetLogLevel(logLevel.first);
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine();
                if(ImGui::Button("Show Logs")){
                    bShowLogs = true;
                    ImGui::SetWindowFocus("Logger");
                }

                ImGui::SeparatorText("Gui");

                const char* previewTxt = themeID==ofxSA_GUI_THEME_LIGHT?"Light":"Dark";
#ifdef ofxSA_GUI_THEME_CUSTOM_FILE
                if(themeID==ofxSA_GUI_THEME_CUSTOM) previewTxt = ofxSA_GUI_THEME_CUSTOM_NAME;
#endif
                if(ImGui::BeginCombo("Theme", previewTxt )){
                    if(ImGui::Selectable("Light", themeID==ofxSA_GUI_THEME_LIGHT) ){
                        themeID = ofxSA_GUI_THEME_LIGHT;
                        loadImGuiTheme();
                    }
                    if(ImGui::Selectable("Dark", themeID==ofxSA_GUI_THEME_DARK) ){
                        themeID = ofxSA_GUI_THEME_DARK;
                        loadImGuiTheme();
                    }
                    if(ImGui::Selectable("ImGui Default", themeID==ofxSA_GUI_THEME_IMGUI_DEFAULT) ){
                        themeID = ofxSA_GUI_THEME_IMGUI_DEFAULT;
                        loadImGuiTheme();
                    }
#ifdef ofxSA_GUI_THEME_CUSTOM_FILE
                    if(ImGui::Selectable(ofxSA_GUI_THEME_CUSTOM_NAME, themeID==ofxSA_GUI_THEME_CUSTOM) ){
                        themeID = ofxSA_GUI_THEME_CUSTOM;
                        loadImGuiTheme();
                    }
#endif
                    ImGui::EndCombo();
                }
                if(ImGui::BeginMenu("Gui-Advanced")){
                    ImGui::SeparatorText("ImGui.Configuration");
                    ImGui::BeginDisabled(); // tmp, Fixme: Causes crashes swiching from a menu on window-focused-query, probably an imgui glfw backend bug
                    ImGui::CheckboxFlags("Viewports", &ImGui::GetIO().ConfigFlags, ImGuiConfigFlags_ViewportsEnable);
                    ImGui::EndDisabled();
                    ImGui::CheckboxFlags("Gamepad Navigation", &ImGui::GetIO().ConfigFlags, ImGuiConfigFlags_NavEnableGamepad);
                    ImGui::CheckboxFlags("Keyboard Navigation", &ImGui::GetIO().ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
                    ImGui::Checkbox("Draw software cursor", &ImGui::GetIO().MouseDrawCursor);

                    ImGui::EndMenu();
                }

                // Todo : mouse pointer hide/show/auto options ?

#ifdef ofxSA_DEBUG
#   ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
                ImGui::SeparatorText("ofxTimeMeasurements");
                bool tsEnabled = TIME_SAMPLE_GET_ENABLED();
                if(ImGui::Checkbox("Show TimeMeasurements", &tsEnabled)){
                    TIME_SAMPLE_SET_ENABLED(tsEnabled);
                }
                // 	TIME_MEASUREMENTS_TOP_LEFT, TIME_MEASUREMENTS_TOP_RIGHT, TIME_MEASUREMENTS_BOTTOM_LEFT, TIME_MEASUREMENTS_BOTTOM_RIGHT
                static const char* tsPosOptions[] = {
                    "Top Left",
                    "Top Right",
                    "Bottom Left",
                    "Bottom Right"
                };
                ofxTMDrawLocation tsCurPos = TIME_SAMPLE_GET_INSTANCE()->getDrawLocation();
                if(ImGui::BeginCombo("Draw location", tsPosOptions[tsCurPos])){
                    if(ImGui::Selectable(tsPosOptions[TIME_MEASUREMENTS_TOP_LEFT])){
                        TIME_SAMPLE_SET_DRAW_LOCATION(TIME_MEASUREMENTS_TOP_LEFT);
                    }
                    if(ImGui::Selectable(tsPosOptions[TIME_MEASUREMENTS_TOP_RIGHT])){
                        TIME_SAMPLE_SET_DRAW_LOCATION(TIME_MEASUREMENTS_TOP_RIGHT);
                    }
                    if(ImGui::Selectable(tsPosOptions[TIME_MEASUREMENTS_BOTTOM_LEFT])){
                        TIME_SAMPLE_SET_DRAW_LOCATION(TIME_MEASUREMENTS_BOTTOM_LEFT);
                    }
                    if(ImGui::Selectable(tsPosOptions[TIME_MEASUREMENTS_BOTTOM_RIGHT])){
                        TIME_SAMPLE_SET_DRAW_LOCATION(TIME_MEASUREMENTS_BOTTOM_RIGHT);
                    }
                    ImGui::EndCombo();
                }

                if(ImGui::Button("Sync FPS with document")){
                    TIME_SAMPLE_SET_FRAMERATE(ofGetTargetFrameRate());
                }
#   endif
#endif

                ImGui::EndMenu();
            }

            ImGui::Dummy({5,5});
            ImGui::Separator();
            if( ImGui::MenuItem( "Exit", SHORTCUT_FUNC "+Q" ) ){
                // Alt quits immediately
                if(ImGui::IsKeyDown(ImGuiMod_Alt)){
                    ofExit();
                }
                // Exit in the update cycle, prevents crashes in some cases by letting the draw() finish.
                else {
                    bExitOnNextUpdate = true;
                }
            }
            ImGui::EndMenu();
        } // EndMenu( ENIGMATIK_WINDOW_TITLE )

        ImGui::Text("\t\t");
        if(ImGui::BeginMenu("Status")){

            // Visualise / Handle app state
//                ImGui::Dummy({1,10});
//                //ImGui::Text("Application state");
//                if(ImGui::BeginCombo("Application state", getAppStateName(appState, "404").c_str() )){
//                    for(const auto& state : appStates){
//                        if(ImGui::Selectable(state.second, appState == state.first ) ){
//                            setAppState(state.first);
//                        }
//                    }
//                    ImGui::EndCombo();
//                }

            ImGui::Dummy({1,10});
            ImGui::SeparatorText("Runtime");
            //ImGui::Indent();
            ImGui::Text("FPS        : %.0f (target: %.0f)", ofGetFrameRate(), ofGetTargetFrameRate() );
            ImGui::SameLine();
            static int newTargetFPS = ofGetTargetFrameRate();
            if(ImGui::Button("Change##fps")){
                newTargetFPS = ofGetTargetFrameRate();
                ImGui::OpenPopup("FPSChanger");
            }
            if(ImGui::BeginPopup("FPSChanger")){
                ImGui::InputInt("FPS", &newTargetFPS);
                if(ImGui::Button("Apply")){
                    ImGui::CloseCurrentPopup();
                    ofSetFrameRate(newTargetFPS);

                    // Sync
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
                    TIME_SAMPLE_SET_FRAMERATE(newTargetFPS);
#endif
                }

                // VSync setter (OF provides no getter...)
                ImGui::Text("V-Sync : ");
                ImGui::SameLine();
                if(ImGui::Button("Disable")){
                    ofSetVerticalSync(false);
                }
                ImGui::SameLine();
                if(ImGui::Button("Enable")){
                    ofSetVerticalSync(true);
                }
                ImGui::EndPopup();
            }
            ImGui::PlotHistogram("FPS", FPSHistory, ofxSA_FPS_HISTORY_SIZE, 0, NULL, 0.f, 70.f, ImVec2(0,ImGui::GetTextLineHeight()*2));

            ImGui::Text("Uptime     : %.1f seconds", ofGetElapsedTimef() );
            ImGui::Text("Resolution : %i x %i (ratio %.2f)", ofGetWindowWidth(), ofGetWindowHeight(), (((float)ofGetWindowWidth())/ofGetWindowHeight()) );
            ImGui::SameLine();
            static int newResolution[2] = {0, 0};
            if(ImGui::Button("Change##resolution")){
                newResolution[0] = ofGetWindowWidth();
                newResolution[1] = ofGetWindowHeight();
                ImGui::OpenPopup("ResolutionChanger");
            }
            if(ImGui::BeginPopup("ResolutionChanger")){
                ImGui::InputInt("Width", &newResolution[0]);
                ImGui::InputInt("Height", &newResolution[1]);
                if(ImGui::Button("Apply")){
                    ImGui::CloseCurrentPopup();
                    ofSetWindowShape(newResolution[0], newResolution[1]);
                }

                // VSync setter (OF provides no getter...)
                ImGui::Text("V-Sync : ");
                ImGui::SameLine();
                if(ImGui::Button("Disable")){
                    ofSetVerticalSync(false);
                }
                ImGui::SameLine();
                if(ImGui::Button("Enable")){
                    ofSetVerticalSync(true);
                }
                ImGui::EndPopup();
            }

#ifdef ofxSA_BACKGROUND_CLEARING_GUI
            ImGui::Checkbox("##EnableBGClearing", &bEnableClearing);
            ImGui::SameLine();
            ImGui::ColorEdit4("Background clearing", &bgClearColor[0]);
#endif    
#ifdef ofxSA_BACKGROUND_FADING_GUI
            ImGui::Checkbox("##EnableBGFading", &bEnableFading);
            ImGui::SameLine();
            ImGui::ColorEdit4("Background fading", &bgFadeColor[0]);
#endif

#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
            canvas.drawGuiSettings();
//            ImGui::Text("FPS: %.0f", blend2d.getFps() );
//            ImGui::Text("Timeframe: %.3f sec", blend2d.getSyncTime() );
//            ImGui::PlotHistogram("##blend2d_fps_histogram", &blend2d.getFpsHist(), ofxBlend2D_FPS_HISTORY_SIZE, 0, NULL, 0.f, 75.f, ImVec2(0,30));
#endif
            //ImGui::Unindent();

            //
            ImGui::EndMenu();
        } // end Status menu

        if(ImGui::BeginMenu("Document")){
            ImGui::SeparatorText("Open Document");
            ImGui::TextDisabled("%s", savePath.c_str());
            ImGui::SameLine();
            ImGui::Text("%s", saveName.c_str());
            ImGui::Separator();

#ifdef ofxSA_XML_FOLDER
            static bool reloadFolder = true;
#endif
            if( ImGui::MenuItem( "Save", SHORTCUT_FUNC "+S" ) ){
                saveXmlSettings();
#ifdef ofxSA_XML_FOLDER
                reloadFolder = true;
#endif
            }
            const bool bExists = ofFile(savePath+saveName).exists();
            if(!bExists) ImGui::BeginDisabled();
            if( ImGui::MenuItem( "Load", SHORTCUT_FUNC "+L" ) ){
                loadXmlSettings();
            }
            if(!bExists) ImGui::EndDisabled();

#ifdef ofxSA_XML_FOLDER
            static ofDirectory saveDir;
            static bool checkNewFileName = true;
            if(reloadFolder){
                saveDir.allowExt("xml");
                saveDir.listDir(savePath);
                reloadFolder=false;
                checkNewFileName=true;
            }
            static char newFileName[128];

            if(checkNewFileName){
                // Load initial value ?
                if(strlen(newFileName)==0){
                    saveName.copy(newFileName, IM_ARRAYSIZE(newFileName));
                }
                std::string baseName=newFileName;
                auto extensionPos = baseName.find_last_of(".");
                if(extensionPos==baseName.npos){
                    baseName = baseName.substr(0, extensionPos);
                }

                auto incrementPos = baseName.find_last_of("_");
                unsigned int increment=0;
                if(incrementPos<extensionPos){
                    auto incrementStrVal = baseName.substr(incrementPos+1, extensionPos);
                    increment=ofToInt(incrementStrVal);
                    baseName = baseName.substr(0, incrementPos);
                }
                else {
                    baseName = baseName.substr(0, extensionPos);
                }

                // Ensure default name
                if(baseName.empty()){
                    baseName = "Settings";
                }

                bool firstRun = true;
                while(firstRun || ofFile(newFileName).exists()){
                    std::string newName=baseName;

                    // Add next increment
                    if(increment>=1){
                        newName += "_";
                        newName += ofToString(increment);
                    }
                    // Add extension
                    newName += ".xml";
                    strncpy(newFileName, "\0", IM_ARRAYSIZE(newFileName)-1);
                    newName.copy(newFileName, IM_ARRAYSIZE(newFileName));

                    // Increment
                    firstRun = false;
                    increment++;
                }
                checkNewFileName = false;
            }

            if(!saveDir.exists() || !saveDir.isDirectory()){
                ImGui::TextColored( ImVec4(255,0,0,200), "Warning: the folder %s doesn't exist!", saveDir.path().c_str());
            }
            if( ImGui::BeginMenu("Save as...") ){
                ImGui::SeparatorText("As a new File");

                if(ImGui::InputText("File Name", newFileName, IM_ARRAYSIZE(newFileName), ImGuiInputTextFlags_EnterReturnsTrue)){
                    checkNewFileName = true;
                }

                if(saveDir.size()>0){
                    ImGui::SeparatorText("As an existing file...");
                    for(auto& file : saveDir){
                        if(ImGui::Selectable(file.getFileName().c_str(), false, ImGuiSelectableFlags_NoAutoClosePopups)){
                            std::strncpy(newFileName, "\0", IM_ARRAYSIZE(newFileName)-1);
                            file.getFileName().copy(newFileName, IM_ARRAYSIZE(newFileName));
                        }
                    }
                }

                ImGui::Dummy({5,5});
                ImGui::Separator();
                static bool bLoadNewFile;
                ImGui::Checkbox("Save as a copy // todo", &bLoadNewFile);
                ImGuiEx::ShowHelpMarker("To continue working in the current file");
                if(ImGui::Button("Save")){
                    saveXmlSettings(newFileName);
                    ImGui::CloseCurrentPopup();
                    reloadFolder = true;
                }
                ImGui::EndMenu();
            }

            // Load menu
            const bool bFolderExists = saveDir.exists();
            if(!bFolderExists) ImGui::BeginDisabled();
            if( ImGui::BeginMenu("Load...") ){

                ImGui::SeparatorText("Files");
                ImGui::SameLine();
                if(ImGui::SmallButton("Reload")){
                    reloadFolder = true;
                }

                if(saveDir.size()<=0){
                    ImGui::Text("There are no files yet...");
                }
                else for(auto& file : saveDir){
                    if(ImGui::MenuItem(file.getFileName().c_str())){
                        loadXmlSettings(file.getFileName());
                    }
                }
                ImGui::EndMenu();
            }
            if(!bFolderExists) ImGui::EndDisabled();
#endif
            ImGui::EndMenu();
        }

        // Any Modules Enabled ?
#ifdef ofxSA_HAS_MODULES_MENU
        if(ImGui::BeginMenu("Modules")){
#   ifdef ofxSA_QUADWRAPPER_ENABLE
            if(ImGui::BeginMenu("Quad Wrapper")){
                ImGui::Dummy({ofxSA_UI_MARGIN, ofxSA_UI_MARGIN});
                ImGui::TextDisabled("Experimental feature !");
                ImGui::SeparatorText("Quad Wrapping");
                ImGui::Checkbox("Enable Quad Wrapping", &bEnableQuadWarper);
                static bool bEnableQuadEditing = false;
                if(ImGui::Checkbox("Enable Editing Quads", &bEnableQuadEditing)){
                    quadWarper.activate(bEnableQuadEditing); // to enable event listeners for easily configuring the quads
                }
                ImGui::Text("Quads :");
                ImGui::SameLine();
                if(ImGui::Button("Reset")){
                    auto docRect = getDocumentSize();
                    quadWarper.setup(0, 0, docRect.getWidth(), docRect.getHeight());
                }
                ImGui::SameLine();
                if(ImGui::Button("Save")){
                    quadWarper.save();
                }
                ImGui::SameLine();
                if(ImGui::Button("Load")){
                    quadWarper.load();
                }
                static std::pair<const char*, ofxGLWarper::CornerLocation> cornerModes[4] = {
                    { "TL", ofxGLWarper::TOP_LEFT},
                    { "TR", ofxGLWarper::TOP_RIGHT},
                    { "BR", ofxGLWarper::BOTTOM_RIGHT},
                    { "BL", ofxGLWarper::BOTTOM_LEFT}
                };
                for(auto& cornerMode : cornerModes){
                    ImVec2 corner = quadWarper.getCorner(cornerMode.second);
                    if(ImGuiEx::DragPad2(cornerMode.first, corner)){
                        quadWarper.setCorner(cornerMode.second, corner);
                    }
                }
                ImGui::EndMenu();
            }
#   endif

#   ifdef ofxSA_SYPHON_OUTPUT
            if(ImGui::BeginMenu("Syphon")){
                ImGui::SeparatorText("Server");
                ImGui::Checkbox("Enable syphon output", &bEnableSyphonOutput);
#if ofxSA_NEWFRAME_FLAGGER == 1
                ImGui::Checkbox("Only publish new frames", &bRecordersOnlyPublishNewFrames);
#endif
                static char syphonServerName[ofxSA_SYPHON_NAME_MAXLEN];//
                if(ImGui::InputText("Server Name", &syphonServerName[0], ofxSA_SYPHON_NAME_MAXLEN, ImGuiInputTextFlags_EnterReturnsTrue)){
                     syphonServer.setName(syphonServerName);
                }
                if(!ImGui::IsItemActive()){ // Sync title when not editing
                    std::strncpy(syphonServerName, syphonServer.getName().c_str(), ofxSA_SYPHON_NAME_MAXLEN-1);
                }

                // todo: show dimensions, fps, etc.
                ImGui::SeparatorText("Status");
#       ifdef ofxSA_CANVAS_OUTPUT_ENABLE
                ofTexture& tex = canvas.fbo.getTexture();
                if(tex.isAllocated())
                    ImGui::TextDisabled("Resolution: %.0f x %.0f", tex.getWidth(), tex.getHeight());
                else
                    ImGui::TextDisabled("Resolution: [unknown]");
#       else
                ImGui::TextDisabled("Resolution: %u x %u", ofGetWidth(), ofGetHeight());
#       endif
#       ifdef ENABLEME_WHEN_OFXSYPHON_UPSTREAM_UPDATED
                ImGui::TextDisabled("hasClients: %s", syphonServer.hasClients()?"Yes":"No");
#       endif

                ImGui::EndMenu();
            }
#   endif

#   ifdef ofxSA_TEXRECORDER_ENABLE
            if(ImGui::BeginMenu("Texture Recorder")){
                //ImGui::CollapsingHeader("Texture Recorder", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf);

                // Destination
                ImGui::SeparatorText("Destination");
                static char newFileName[256];
                if(ImGui::InputText("Target name", newFileName, IM_ARRAYSIZE(newFileName), ImGuiInputTextFlags_EnterReturnsTrue)){
                    recordingTargetName = newFileName;
                    std::cout << "NewName=" << recordingTargetName << std::endl;
                }
                if(!ImGui::IsItemActive()){ // checkme : and just been disabled ? or wasActive...
                    std::strncpy(newFileName, recordingTargetName.c_str(), IM_ARRAYSIZE(newFileName)-1);
                }

#if ofxSA_NEWFRAME_FLAGGER == 1
                ImGui::Checkbox("Only publish new frames", &bRecordersOnlyPublishNewFrames);
#endif

                // Cur/Next filename
                if(isRecordingCanvas){
                    ImGui::TextDisabled("Current : %s", curRecordingName.c_str());
                }
                else {
                    const std::string nextRec = getNextRecordingName();//getNewFileName(ofxSA_TEXRECORDER_DEFAULT_OUTPUT_FOLDER, recordingTargetName.c_str(), extension, true, "_");
                    ImGui::TextDisabled("Next    : %s", nextRec.c_str());
                }

                // Begin recording
                ImGui::BeginDisabled();
                ImGui::Checkbox("Recording state", &isRecordingCanvas);
                ImGui::EndDisabled();
                ImGui::SameLine();
                if(!isRecordingCanvas){
                    if(ImGui::Button("Start##startRecordCanvas")){
                        startRecordingCanvas();
                    }
                }
                else {
                    if(ImGui::Button("Stop##startRecordCanvas")){
                        stopRecordingCanvas();
                    }
                }


                ImGui::SeparatorText("Settings");

                // Time settings
#   ifdef ofxSA_TIMELINE_ENABLE
                ImGui::Checkbox("Stop on loop", &bRecorderStopOnLoop);
                const auto& tl = ofxSA_TIMELINE_GET(timeline);
                ImGui::DragInt2("Frame range to record", &recordFrameRange[0], 1, -1, tl.getTotalFrames());
                const unsigned int fromRec = recordFrameRange[0]>0 ? recordFrameRange[0] : 0;
                const unsigned int toRec = recordFrameRange[1]>0 ? recordFrameRange[1] : tl.getTotalFrames();
                ImGui::TextDisabled("Record Start : %fs", fromRec*(1.f/tl.getFps()));
                ImGui::TextDisabled("Record End   : %fs", toRec*(1.f/tl.getFps()));
                ImGui::TextDisabled("Record Length: %fs", (toRec-fromRec)*(1.f/tl.getFps()));
#   else
                const float frameTime = 1.f/ofGetTargetFrameRate();
                ImGui::DragFloat("Seconds to record", &recordLengthSeconds, frameTime, 0.f, 60*60*24.f, "%.3f sec");
                if(recordLengthSeconds<=0){
                    ImGui::TextDisabled("Duration : Infinite");
                }
                else {
                    ImGui::TextDisabled("Duration : %.2fsec, ±%.0f frames @ %.0f fps", recordLengthSeconds, recordLengthSeconds/frameTime, ofGetTargetFrameRate());
                }
                if(isRecordingCanvas && recordStartSeconds>=0 && recordLengthSeconds>0){
                    ImGui::TextDisabled("Progress: %.0f%%", (ofGetElapsedTimef()-recordStartSeconds)/recordLengthSeconds*100.f);
                }
#   endif

                // Mode
                if(isRecordingCanvas) ImGui::BeginDisabled();
                static const char* recModes[] = {
                    "ffmpeg",
                    "PNG"
                };
                int curMode = texRecorderMode;
                if(ImGui::Combo("Recording Mode", &curMode, recModes, IM_ARRAYSIZE(recModes))){
                    texRecorderMode = static_cast<ofxSimpleApp::TexRecorderMode_>(curMode);
                }
                if(isRecordingCanvas) ImGui::EndDisabled();

                // PNG options
                if(texRecorderMode==TexRecorderMode_PNG){
                    ImGui::PushTextWrapPos(300);// Prevents GUI jump on menu first frame appearing
                    ImGui::TextWrapped("In PNG mode, sequential PNG files will be written to the destination folder.\n One folder per recording.");
                    ImGui::PopTextWrapPos();

                    ImGui::SeparatorText("PNG Settings");
                    ImGui::Checkbox("Threaded recording", &bThreadedRecording); // threaded is only for PNG
                }
                else if(texRecorderMode==TexRecorderMode_FFMPEG){
                    ImGui::SeparatorText("FFMPEG Settings");
                    if(ImGui::BeginCombo("Codec", selectedCodec)){
                        for(auto& c : ffmpegRecordingFormats){
                            if(ImGui::Selectable(c.first, selectedCodec && (strcmp(selectedCodec, c.first)==0), ImGuiSelectableFlags_AllowOverlap)){
                                selectedCodec = c.first;
                                m_Recorder.setVideoCodec(c.first);
                            }
                            ImGui::SameLine();
                            ImGui::TextDisabled("%s", c.second);
                        }
                        ImGui::EndCombo();
                    }
                    static unsigned int bitrateMin = 1u, bitrateMax = 100000u;
                    if(ImGui::DragScalar("Video Bitrate", ImGuiDataType_U32, &bitrateVideo, 1.f, &bitrateMin, &bitrateMax, "%u")){
                        m_Recorder.setBitRate(bitrateVideo);
                    }

                    ImGui::BeginDisabled(); // not yet implemented !
                    ImGui::Checkbox("Record Audio", &bRecordAudioToo);
                    ImGui::EndDisabled();

                    // Audio settings
                    if(bRecordAudioToo){
                        static unsigned int bitrateAudioMax = 320u;
                        if(ImGui::DragScalar("Audio Bitrate", ImGuiDataType_U32, &bitrateVideo, 1.f, &bitrateMin, &bitrateAudioMax, "%u")){
                            m_Recorder.setBitRateAudio(bitrateVideo);
                        }
                    }
                }

                // FFMpeg details
                if(texRecorderMode==TexRecorderMode_FFMPEG){
                    ImGui::SeparatorText("FFMPEG Status");
                    ImGui::Text("FFmpeg   : %s", m_Recorder.getFFmpegPath().c_str());
                    ImGui::Text("BitrateV : %u", m_Recorder.getBitRate());
                    ImGui::Text("BitrateA : %u", m_Recorder.getBitRateAudio());
                    ImGui::Text("FPS      : %.2f", m_Recorder.getFps());
                    ImGui::Text("Width    : %.0f", m_Recorder.getWidth());
                    ImGui::Text("Height   : %.0f", m_Recorder.getHeight());
                    ImGui::Text("Duration : %.2f / %.2f", m_Recorder.getCaptureDuration(), m_Recorder.getRecordedDuration());
                    ImGui::Text("File     : %s", m_Recorder.getOutputPath().c_str());
                    ImGui::Text("Codec    : %s", m_Recorder.getVideoCodec().c_str());
                }

                ImGui::EndMenu(); // texrecorder menu
            }
#   endif // TEXRECORDER

#   ifdef ofxSA_NDI_SENDER_ENABLE
            if(ImGui::BeginMenu("NDI Sender")){
                ImGui::SeparatorText("Server");
                bool bNdiEnabled = ndiSender.SenderCreated();
                ImGuiEx::ofxNdiSenderSetup(ndiSender);
#if ofxSA_NEWFRAME_FLAGGER == 1
                ImGui::Checkbox("Only publish new frames", &bRecordersOnlyPublishNewFrames);
#endif

                ImGui::SeparatorText("Runtime settings");
                ImGuiEx::ofxNdiSenderSettings(ndiSender);

                ImGui::SeparatorText("NDI Status");
                ImGuiEx::ofxNdiSenderStatusText(ndiSender);

                ImGui::EndMenu();
            }
#   endif

            ImGui::EndMenu(); // Modules Menu
        }
#endif // ofxSA_HAS_MODULES_MENU

        // View menu
        if(ImGui::BeginMenu("View")){
            if(ImGui::BeginMenu("Gui")){
                if(ImGui::MenuItem("Hide GUI", SHORTCUT_FUNC "+G")){
                    bShowGui = false;
                    updateCursorForGui();
                }
                if(ImGui::MenuItem("Toggle full screen", SHORTCUT_FUNC "+F")){
                    ofToggleFullscreen();
                    onImguiViewportChange();
                }
                ImGui::EndMenu();
            }

            // Debug
            if(ImGui::BeginMenu("Debug")){
                if(ImGui::Checkbox("Show log window", &bShowLogs) && bShowLogs){
                    ImGui::SetWindowFocus("Logger");
                }

#ifdef ofxSA_DEBUG
#   ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
                ImGui::SeparatorText("ofxTimeMeasurements");
                bool tsEnabled = TIME_SAMPLE_GET_ENABLED();
                if(ImGui::Checkbox("Show TimeMeasurements", &tsEnabled)){
                    TIME_SAMPLE_SET_ENABLED(tsEnabled);
                }
#       if defined(ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY) && defined(ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY_NAME)
                ImGui::SameLine();
                ImGui::TextDisabled("%s", ofToString(ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY_NAME).c_str());
#       endif
#   endif

                ImGui::SeparatorText("ImGui Dev");
                ImGui::Checkbox("Show ImGui Metrics", &bShowImGuiMetrics);
                ImGui::Checkbox("Show ImGui Debug Window", &bShowImGuiDebugLog);
                ImGui::Checkbox("Show ImGui Demo Window", &bShowImGuiDemo);
#endif
                ImGui::EndMenu();
            }

#ifdef ofxSA_TIMELINE_ENABLE
            if(ImGui::Checkbox("Show Timeline", &bShowTimeClockWindow) && bShowTimeClockWindow){
                ImGui::SetWindowFocus("Timeline");
            }
#endif

            ImGui::EndMenu();
        }

        // Spacer for custom menu items (after)
        ImGui::Text("\t");

        // Update menu height
        const float curMenuHeight = ImGui::GetWindowSize().y;
        if(imguiMenuHeight != curMenuHeight){
            imguiMenuHeight = curMenuHeight;
        }
    }

    ImGui::EndMainMenuBar();
}

void ofxSimpleApp::ImGuiDrawAboutWindow(){

    // Show About Window ?
    if(bShowAboutWindow){
        ImGui::SetNextWindowSize(ImVec2(400,500), ImGuiCond_Appearing );
        if( ImGui::Begin("About", &bShowAboutWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking ) ){

            // About Header
            ImGui::Dummy(ImVec2(0,50));
            ImGui::Text(  ofxSA_APP_NAME );
            if(ofxSA_APP_COPYRIGHT_START_YEAR != 0 && ofxSA_APP_COPYRIGHT_START_YEAR != curYear){
                ImGui::Text("Copyright %i-%i %s", ofxSA_APP_COPYRIGHT_START_YEAR, curYear, ofxSA_APP_AUTHOR);
            }
            else {
                ImGui::Text("Copyright %i %s", curYear, ofxSA_APP_AUTHOR);
            }

            ImGui::Text("Version %d.%d.%d", ofxSA_APP_VERSION_MAJOR, ofxSA_APP_VERSION_MINOR, ofxSA_APP_VERSION_PATCH );
            ImGui::Dummy(ImVec2(0,50));
            //ImGui::Separator();

            // About Tabs
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)){

                if( ImGui::BeginTabItem("About") ){
                    ImGui::Spacing();
                    ImGui::TextWrapped("%s is a project made by %s.\n\n", ofxSA_APP_NAME, ofxSA_APP_AUTHOR);

#ifdef ofxSA_APP_ABOUT
                    ofxSA_APP_ABOUT
#endif

#ifdef ofxSA_APP_AUTHOR_WEBSITE
                    ImGui::TextWrapped("More information: %s", ofxSA_APP_AUTHOR_WEBSITE);
                    if(ImGui::IsItemClicked()){
                        ofLaunchBrowser(ofxSA_APP_AUTHOR_WEBSITE);
                    }
#endif
                    ImGui::Spacing();
                    ImGui::EndTabItem();
                } // End About

                if (ImGui::BeginTabItem("Build Info")){

                    //ImGui::TextWrapped("Feel free to include the following information in bug reports.");
                    bool copy_to_clipboard = ImGui::Button("Copy to clipboard");

                    ImGui::Spacing();
                    ImGui::BeginChild(ImGui::GetID("Build Configuration"), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 18), ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoMove);
                    if (copy_to_clipboard){
                        ImGui::LogToClipboard();
                    }
#ifdef DEBUG
#define BUILDVARIANT "Debug"
#else
#define BUILDVARIANT "Release"
#endif
                    ImGui::Text( "Build variant: %s", BUILDVARIANT );
                    ImGui::Separator();
                    // System Info
#ifdef _WIN32
                    ImGui::Text("define: _WIN32");
#endif
#ifdef _WIN64
                    ImGui::Text("define: _WIN64");
#endif
#ifdef __linux__
                    ImGui::Text("define: __linux__");
#endif
#ifdef __APPLE__
                    ImGui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
                    ImGui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef __MINGW32__
                    ImGui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
                    ImGui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
                    ImGui::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
                    ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
                    ImGui::Separator();
                    ImGui::Text("openFrameworks v%d.%d.%d", OF_VERSION_MAJOR, OF_VERSION_MINOR, OF_VERSION_PATCH);
                    ImGui::Separator();

                    ImGui::SeparatorText("ofxSimpleApp Settings");
#if (ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML)
                    ImGui::Text("ofxSA XML engine : PugiXML %i.%i.%i (%i)", ofxPugiXml::versionMajor(), ofxPugiXml::versionMinor(), ofxPugiXml::versionPatch(), PUGIXML_VERSION);
                    ImGui::Separator();

#elif (ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_TINYXML)
                    ImGui::Text("ofxSA XML engine : TinyXML %i.%i.%i", TIXML_MAJOR_VERSION, TIXML_MINOR_VERSION, TIXML_PATCH_VERSION);
#else
                    ImGui::Text("ofxSA XML engine : Unknown");
#endif
                    ImGui::Separator();


                    // Todo: Split Compile info and runtime info

                    // Add HW information ?
                    // With sysctl header : https://stackoverflow.com/questions/853798/programmatically-get-processor-details-from-mac-os-x
                    // Win+Linux : https://github.com/SaulBerrenson/sys_info
                    // List threads from PID : `ps -M 9034`
                    // HWinfo : https://github.com/lfreist/hwinfo
                    // Foreign processes too ? https://github.com/time-killer-games/xproc
                    // Or via getrusage ? https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/getrusage.2.html

                    // GPU
                    ImGui::Text("GPU Information:");
                    static bool isProgrammable = ofIsGLProgrammableRenderer();
                    static int major = ofGetGLRenderer()->getGLVersionMajor();
                    static int minor = ofGetGLRenderer()->getGLVersionMinor();
#ifdef TARGET_OPENGLES
                    ImGui::Text("GL ES %d.%d (fixed pipeline)", major, minor);
#else
                    ImGui::Text("GL SL %d.%d (programmable)", major, minor);
#endif
                    // Note: ofGLSLVersionFromGL is very outdated... we do it manually here.
                    const char* glsl_name = "Unknown";
                    const char* glsl_version = "Unknown";
                    // ofGLSLVersionFromGL is very outdated... we do it manually here.
                    // See imgui_impl_opengl3.cpp
                    //----------------------------------------
                    // OpenGL    GLSL      GLSL
                    // version   version   string
                    //----------------------------------------
                    //  2.0       110       "#version 110"
                    //  2.1       120       "#version 120"
                    //  3.0       130       "#version 130"
                    //  3.1       140       "#version 140"
                    //  3.2       150       "#version 150"
                    //  3.3       330       "#version 330 core"
                    //  4.0       400       "#version 400 core"
                    //  4.1       410       "#version 410 core"
                    //  4.2       420       "#version 410 core"
                    //  4.3       430       "#version 430 core"
                    //  ES 2.0    100       "#version 100"      = WebGL 1.0
                    //  ES 3.0    300       "#version 300 es"   = WebGL 2.0
                    //----------------------------------------


#ifdef TARGET_OPENGLES
                    if( isProgrammable ){
                        if( major==2 ){
                            glsl_version = "#version 100";
                            glsl_name = "GL ES 2";
                        }
                        else if( major==3 ){ // Note: not yet available in oF !!!
                            glsl_version = "#version 300 es";
                            glsl_name = "GL ES 3";
                        }
                    }
                    else {
                        glsl_name = "GL ES 1";
                        glsl_version = "none";
                    }
#else
                    if( isProgrammable ){
                        if( major==3 ){
                            glsl_name = "GLSL 3";

                            if( minor==0 )      glsl_version="#version 130";
                            else if( minor==1 ) glsl_version="#version 140";
                            else if( minor==2 ) glsl_version="#version 150";
                            else if( minor==3 ) glsl_version="#version 330 core";
                        }
                        else if( major==4 ){
                            glsl_name = "GLSL 4";
                            if( minor==0 )      glsl_version="#version 400 core";
                            else if( minor==1 ) glsl_version="#version 410 core";
                            else if( minor==2 ) glsl_version="#version 420 core";
                            else if( minor==3 ) glsl_version="#version 430 core";
                        }
                    }
                    else {
                        glsl_name = "GL 2";
                        glsl_version = "none";
                    }
#endif
                    ImGui::Text("Shaders: %s (%s)", glsl_name, glsl_version);

                    // Some info is double, but needs to be tested what works best in multiple environments
                    ImGui::Text( "Vendor : %s", glGetString(GL_VENDOR) );
                    ImGui::Text( "GPU    : %s", glGetString(GL_RENDERER) );
                    ImGui::Text( "OpenGL ver. %s", glGetString(GL_VERSION) ); // alt: glGetString(GL_MAJOR_VERSION), glGetString(GL_MINOR_VERSION)
                    ImGui::Text( "GLSL ver. %s / %s", glGetString(GL_SHADING_LANGUAGE_VERSION), ofGLSLVersionFromGL(major, minor).c_str() );

#ifdef GL_MAX_COLOR_ATTACHMENTS
                    GLint maxAttach = 0;
                    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
                    ImGui::Text("Max textures        : %i", maxAttach);
#endif
#ifdef GL_MAX_DRAW_BUFFERS
                    GLint maxFbos = 0;
                    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxFbos);
                    ImGui::Text("Max FBOs            : %i", maxFbos);
#endif
#ifdef GL_MAX_VERTEX_ATTRIBS
                    GLint maxVertexAttrs = 0;
                    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttrs);
                    ImGui::Text("Max vertex attrs    : %i", maxVertexAttrs);
#endif
#ifdef GL_MAX_TEXTURE_IMAGE_UNITS
                    GLint maxIUs = 0;
                    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxIUs);
                    ImGui::Text("Max image units     : %i", maxIUs);
#endif
#ifdef GL_MAX_TEXTURE_COORDS
                    GLint maxTexCoords = 0;
                    glGetIntegerv(GL_MAX_TEXTURE_COORDS, &maxTexCoords);
                    ImGui::Text("Max texcoords       : %i", maxTexCoords);
#endif
#ifdef GL_MAX_COMBINED_DIMENSIONS
                    GLint maxCombinedDimensions = 0;
                    glGetIntegerv(GL_MAX_COMBINED_DIMENSIONS, &maxCombinedDimensions);
                    ImGui::Text("Max combined dimens : %i", maxCombinedDimensions);
#endif
#ifdef GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
                    GLint maxCombinedTexUnits = 0;
                    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTexUnits);
                    ImGui::Text("Max combined images : %i", maxCombinedTexUnits);
#endif
#ifdef GL_MAX_COMBINED_IMAGE_UNIFORMS
                    GLint maxCombinedUIs = 0;
                    glGetIntegerv(GL_MAX_COMBINED_IMAGE_UNIFORMS, &maxCombinedUIs);
                    ImGui::Text("Max comb. uniform textures : %i", maxCombinedUIs);
#endif
#ifdef GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB
                    GLint maxCombinedUIsArb = 0;
                    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &maxCombinedUIsArb);
                    ImGui::Text("Max comb. uniform tex.ARB  : %i", maxCombinedUIsArb);
#endif
#ifdef GL_MAX_FRAGMENT
                    //GLint maxFbos = 0;
                    //glGetIntegerv(GL_MAX_FRAGMENT, &maxAttach);
                    //ImGui::Text("Max vertex texunits     : %i", maxFbos);
#endif
#ifdef GL_MAX_VARYING_FLOATS
                    GLint maxVaryingFloats = 0;
                    glGetIntegerv(GL_MAX_VARYING_FLOATS, &maxVaryingFloats);
                    ImGui::Text("Max varying floats  : %i", maxVaryingFloats);
#endif
#ifdef GL_MAX_VERTEX_UNIFORM_COMPONENTS
                    GLint maxVertexUCs = 0;
                    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUCs);
                    ImGui::Text("Max vert uniforms   : %i", maxVertexUCs);
#endif
#ifdef GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
                    GLint maxFragmentUniforms = 0;
                    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniforms);
                    ImGui::Text("Max frag uniforms   : %i", maxFragmentUniforms);
#endif
#ifdef GL_ATTACHED_SHADERS
                    GLint curAttachedShaders = 0;
                    glGetIntegerv(GL_ATTACHED_SHADERS, &curAttachedShaders);
                    ImGui::Text("Current num. shaders: %i", curAttachedShaders);
#endif
#ifdef GL_ACTIVE_UNIFORMS
                    GLint curUniforms = 0;
                    glGetIntegerv(GL_ACTIVE_UNIFORMS, &curUniforms);
                    ImGui::Text("Active uniforms     : %i", curUniforms);
#endif

//                        GLint maxFbos = 0;
//                        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxAttach);
//                        ImGui::Text("Max GL FBOs     : %i", maxFbos);

// Useful code : https://github.com/SaschaWillems/glCapsViewer/blob/85ee6ab685174f7223cf726e7b1f878d6cac3984/glCapsViewerCore.cpp#L170
#if defined(GL_EXTENSIONS)
ImGui::Separator();
#if defined(GL_NUM_EXTENSIONS) // OpenGL 3+
                    static int numExtensions = 0;
                    std::string extensions = "";
                    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
                    for(int i = 0; i<numExtensions; ++i){
                        if(i!=0) extensions.append(", ");
                        extensions.append( (char*)glGetStringi(GL_EXTENSIONS, i) );
                    }
                    ImGui::TextWrapped( "GL Extensions (%d) :\n%s", numExtensions, extensions.c_str());//, glGetString(GL_EXTENSIONS) );
#else // OpenGL 2, GLES
                    const GLubyte* glExtensions = glGetString(GL_EXTENSIONS);
                    string extensions = reinterpret_cast<const char*>(glExtensions);
                    ImGui::TextWrapped( "GL Extensions :\n%s", extensions.c_str());
#endif
ImGui::Separator();
#endif

                    ImGui::Spacing();

                    if (copy_to_clipboard){
                        ImGui::LogFinish();
                    }
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                } // end build info

                // Runtime
                if( ImGui::BeginTabItem("Runtime") ){


                    // Monitor information
                    ImGui::SeparatorText("Displays");

                    // Courtesy of d3cod3/Mosaic
                    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
                    if(primaryMonitor){
                        // Todo: move to setup ?
                        const GLFWvidmode * mode = glfwGetVideoMode(primaryMonitor); // for monitor resolution
                        bool    isRetina;
                        int     retinaScale;
                        int     pixelScreenScale;
                        float   xScreenContentScale, yScreenContentScale;
                        int     wScreenMM, hScreenMM;
                        int     wRes,hRes;
                        float   monitorContentScale;
                        float   pixelsxMM;
                        float   fontScaling;
                        int                             suggestedFontSize;

                        wRes = mode->width;
                        hRes = mode->height;
                        glfwGetMonitorContentScale(primaryMonitor, &xScreenContentScale, &yScreenContentScale);
                        glfwGetMonitorPhysicalSize(primaryMonitor, &wScreenMM, &hScreenMM);

                        pixelsxMM = mode->height/hScreenMM;
                        suggestedFontSize = static_cast<int>(ofMap(yScreenContentScale,1,8,10,56));

                        // RETINA FIX
                        fontScaling = 1;
                        retinaScale = yScreenContentScale;
                        isRetina = false;
                        if(retinaScale > 1){
                            isRetina = true;
                        }
                        pixelScreenScale = retinaScale*fontScaling;

                        ImGui::Spacing();
                        ImGui::Text("Primary Monitor:");
                        ImGui::Spacing();
                        ImGui::Text("Resolution: %sx%s", ofToString(wRes).c_str(),ofToString(hRes).c_str());
                        ImGui::Text("Physical Dimensions: %sx%s mm", ofToString(wScreenMM).c_str(),ofToString(hScreenMM).c_str());
                        ImGui::Text("Retina Scale ( Retina Screens ): %s",ofToString(retinaScale).c_str());
                        ImGui::Text("Screen Content Scale ( OS managed, accessibility ): %s",ofToString(xScreenContentScale).c_str());
                        ImGui::Text("Pixel density ( Resolution over Physical Size): %s",ofToString(pixelsxMM).c_str());
                        ImGui::Text("Suggested font size: %s",ofToString(suggestedFontSize+(4*retinaScale)).c_str());
                    }

                    ImGui::Spacing();

                    // osx
                    // Required to install cuda devkit : https://developer.nvidia.com/cuda-downloads
                    // Look at : https://github.com/phvu/cuda-smi/blob/master/cuda-smi.cpp

                    // Todo : https://gitlab.artificiel.org/ofxaddons/ofxnvidiasmi
                    // Windows + Linux

                    ImGui::EndTabItem();
                } // End Runtime

                // Credits
                if( ImGui::BeginTabItem("Credits") ){
                    ImGui::Text("Todo...");
                    ImGui::EndTabItem();
                } // End About

                ImGui::EndTabBar();
            } // End Tabs

        } // About window visible
        ImGui::End();
    } // end about window condition
}

#if ofxSA_UI_DOCKING_ENABLE_DOCKSPACES == 1
void ofxSimpleApp::ImGuiDrawDockingSpace(){
    // Make windows transparent, to demonstrate drawing behind them.
    ImGui::PushStyleColor(ImGuiCol_WindowBg , IM_COL32(200,200,200,128)); // This styles the docked windows

    ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_PassthruCentralNode; // Make the docking space transparent
    // Fixes imgui to expected behaviour, having a transparent central node in passthru mode.
    // Alternative: Otherwise add in ImGui::DockSpace() [±line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0,0,0,0));

    dockingFlags |= ImGuiDockNodeFlags_NoDockingOverCentralNode; // Uncomment to always keep an empty "central node" (a visible oF space)
    //dockingFlags |= ImGuiDockNodeFlags_NoTabBar; // Uncomment to disable creating tabs in the main view
    //dockingFlags |= ImGuiDockNodeFlags_AutoHideTabBar;
    //dockingFlags |= ImGuiDockNodeFlags_NoDockingSplitMe;
//    dockingFlags |= ImGuiDockNodeFlags_NoDockingOverMe;

    // Define the ofWindow as a docking space
    ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(0, NULL, dockingFlags); // Also draws the docked windows
    ImGui::PopStyleColor(2);

    ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockNodeID);
    if(dockNode){
        ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(dockNodeID);
        // Verifies if the central node is empty (visible empty space for oF)
        if( centralNode && centralNode->IsEmpty() ){
            ImRect availableSpace = centralNode->Rect();

            // Detect change ?
            if(
                dockingViewport.x != availableSpace.Min.x ||
                dockingViewport.y != availableSpace.Min.y ||
                dockingViewport.width != availableSpace.GetWidth() ||
                dockingViewport.height != availableSpace.GetHeight()
            ){
                // Update viewport
                dockingViewport.x = availableSpace.Min.x;
                dockingViewport.y = availableSpace.Min.y;
                dockingViewport.width = availableSpace.GetWidth();
                dockingViewport.height = availableSpace.GetHeight();

                // Notify
                onImguiViewportChange();
            }

            //availableSpace.Max = availableSpace.Min + ImGui::GetContentRegionAvail();
            //ImGui::GetBackgroundDrawList()->AddRect(availableSpace.GetTL()+ImVec2(8,8), availableSpace.GetBR()-ImVec2(8,8), IM_COL32(255,50,50,255));

            //ImVec2 viewCenter = availableSpace.GetCenter();

            // Depending on the viewports flag, the XY is either absolute or relative to the oF window.
            //if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) viewCenter = viewCenter - ImVec2(ofGetWindowPositionX(),ofGetWindowPositionY());

//            ofPushStyle();
//            ofSetRectMode(OF_RECTMODE_CENTER);
//            ofSetColor(255,0,0,2);
//            ofNoFill();

//            ofDrawRectangle(
//                        viewCenter.x,
//                        viewCenter.y,
//                        availableSpace.GetSize().x-6,
//                        availableSpace.GetSize().y-6
//            );
//            ofNoFill();
//            ofSetColor(255,255,255,30);
//            ofDrawRectangle(
//                        (viewCenter.x),
//                        (viewCenter.y),
//                        (availableSpace.GetSize().x-20)*fmodf(abs(sin(ofGetElapsedTimef())),1.f),
//                        (availableSpace.GetSize().y-20)*fmodf(abs(sin(ofGetElapsedTimef())),1.f)
//            );
//            ofSetRectMode(OF_RECTMODE_CORNER);
//            ofPopStyle();
        }
    }
}

void ofxSimpleApp::onImguiViewportChange(){
    // Update canvas
#   ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    ofRectangle vp = getGuiViewport(false);
    canvas.setViewportRect(vp.width, vp.height, vp.x, vp.y);
#   endif
}
#endif // ofxSA_UI_DOCKING_ENABLE_DOCKSPACES

void ofxSimpleApp::onViewportChange(){
    // Update canvas
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    //ofRectangle vp = getGuiViewport();
    //canvas.setViewportRect(vp.width, vp.height, vp.x, vp.y);
#endif
}

void ofxSimpleApp::onContentResize(unsigned int _width, unsigned int _height){

}

#ifdef ofxSA_TIMELINE_ENABLE
void ofxSimpleApp::ImGuiDrawTimeline(){
    if(bShowTimeClockWindow){
        ofxSA_TIMELINE_GET(timeline).drawImGuiWindow(&bShowTimeClockWindow);
    }
}
#endif

// Viewport handling
//void ofxSimpleApp::updateViewport(){
//    // Set coords to window coords
//    curViewport.x = 0;
//    curViewport.y = 0;
//    curViewport.width = ofGetWidth();
//    curViewport.height = ofGetHeight();

//    // Subtract Menu bar
//    if(bShowGui){
//        float menuHeight = ImGui::GetFrameHeight();
//        curViewport.x += ofGetHeight();
//        curViewport.height -= ofGetHeight();
//    }
//}

// Returns the viewport visible within the GUI
// Note: for the ofAppWindow viewport, use ofGetWindowRect();
ofRectangle ofxSimpleApp::getGuiViewport(bool returnScreenCoords) const {
    // Set coords to window coords
    ofRectangle curViewport = ofGetWindowRect();

    // Subtract gui zones if visible
    if(bShowGui){
#if ofxSA_UI_DOCKING_ENABLE_DOCKSPACES == 1
        // Return visible zone (without docking spaces and menus)
        curViewport = dockingViewport;

        // Make them relative to screen (absolute in fact)
        if(!returnScreenCoords && ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable ){
            curViewport.position.x -= ofGetWindowPositionX();
            curViewport.position.y -= ofGetWindowPositionY();
        }
#else
        // Subtract the ImGui menu bar
        //float menuHeight = ImGui::GetFrameHeight();
        curViewport.x += imguiMenuHeight;
        curViewport.height -= imguiMenuHeight;
#endif
    }

    return curViewport;
}

// Document size : ofAppWindow or canvas size if using canvas.
ofRectangle ofxSimpleApp::getDocumentSize() const {
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    return {0, 0, (float)canvas.getCanvasWidth(), (float)canvas.getCanvasHeight()};
#else
    // Fallback to window size
    return ofGetCurrentViewport();
#endif
}

// Xml settings from the ofxXml object
bool ofxSimpleApp::loadXmlSettings(std::string _fileName){
    if(_fileName.empty()) _fileName = saveName;
    std::string path = savePath+_fileName;
    bool ret = true;
    // Check file existence
    if( !ofFile(path).exists() ){
        // File doesn't exists, ignore
        ofLogVerbose("ofxSimpleApp::loadXmlSettings()") << "The settings file `"<< path << "` doesn't exist yet, nothing to load !";
        return true; // No error, but nothing loaded
    }

    // Load string buffer
#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(ofToDataPath(path, true).c_str());
    bool loaded = false;
    if(!result){
        ofLogWarning("ofxSimpleApp::loadXmlSettings()") << "Pugi error loading the document `"<< path << "`. Error=" << result.description();
        loaded = false;
    }
    else loaded = true;
#else
    xml.clear(); // Ensure to rm garbage
    bool loaded = xml.loadFile("settings.xml");
#endif

    if(!loaded) {
        ofLogError("ofxSimpleApp::loadXmlSettings()") << "Failed loading XML contents from `"<< path << "` !";
        return false;
    }

    // Parse data
#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
    // Load ofxSimpleApp section
    pugi::xml_node ofxSimpleAppSettingsNode = doc.child("ofxSimpleAppSettings");
    if(!ofxSimpleAppSettingsNode){
        ret = false;
        ofLogWarning("ofxSimpleApp::loadXmlSettings()") << "There's no `ofxSimpleAppSettings` section in the document !";
    }
    else {
        // Retrieve data
        if(ofxSimpleApp::ofxSA_retrieveXmlSettings(ofxSimpleAppSettingsNode)){
            ofLogVerbose("ofxSimpleApp::loadXmlSettings()") << "Imported `ofxSimpleAppSettings` section from the document !";
        }
        else {
            ret = false;
            ofLogWarning("ofxSimpleApp::loadXmlSettings()") << "Couldn't parse `ofxSimpleAppSettings` section in the document !";
        }
    }

    // Load custom section
    pugi::xml_node customAppSettingsNode = doc.child(ofxSA_APP_ID);

    // tmp : fallback for previous file load system, broken since commit e92de44a
    if(!customAppSettingsNode){
        ofLogNotice("ofxSimpleApp::loadXmlSettings()") << "Trying to fallback on `" << ofxSA_APP_NAME << "` which was used before !";
        customAppSettingsNode = doc.child(ofxSA_APP_NAME);
    }
    if(!customAppSettingsNode){
        ret = false;
        ofLogWarning("ofxSimpleApp::loadXmlSettings()") << "There's no `" << ofxSA_APP_ID << "` section in the document !";
    }
    else {
        if(retrieveXmlSettings(customAppSettingsNode)){
            ofLogVerbose("ofxSimpleApp::loadXmlSettings()") << "Imported `" << ofxSA_APP_ID << "` section from the document !";
        }
        else {
            ret = false;
            ofLogWarning("ofxSimpleApp::loadXmlSettings()") << "Couldn't parse `" << ofxSA_APP_ID << "` section in the document !";
        }
    }

#else
//    if(!SimpleApp::retrieveXmlSettings()){

//    }
#endif

    if(ret){
        ofLogNotice("ofxSimpleApp::loadXmlSettings()") << "Successfully loaded XML data from the document.";
    }
    else {
        ofLogError("ofxSimpleApp::loadXmlSettings()") << "Failed parsing settings from XML data loaded from the document.";
    }

    return ret;
}

// Saving function
//#include <typeinfo>
bool ofxSimpleApp::saveXmlSettings(std::string _fileName){
    if(_fileName.empty()) _fileName = saveName;
    std::string path = savePath+_fileName;

     bool success = true;

    // Ensure save dir exists
    ofDirectory dir = ofDirectory(ofFilePath::getEnclosingDirectory(path, true));
    if(!dir.exists()){
        success *= dir.create(false);
        if(!success){
            ofLogNotice("ofxSimpleApp::saveXmlSettings()") << "Couldn't create parent folder to save to !";
            return false;
        }
    }

#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
    pugi::xml_document doc;
    pugi::xml_node ofxSimpleAppSettingsNode = doc.append_child("ofxSimpleAppSettings");
    success *= ofxSimpleApp::ofxSA_populateXmlSettings(ofxSimpleAppSettingsNode);

    pugi::xml_node customAppSettingsNode = doc.append_child(ofxSA_APP_ID);
    success *= populateXmlSettings(customAppSettingsNode);  // fixme : maybe this should be 2 different functions. If unimplemented by user, ofxSimpleApp::pxs is called twice ! What if user calls ofxSimpleApp::pxs manually ?

    if(success){
        // Save !
        success *= doc.save_file(ofToDataPath(path).c_str());
    }
    else {
        // todo: failed populating
    }
#else
    xml.clear();
    populateXmlSettings();
    success = xml.saveFile(ofxSA_XML_FILENAME);
#endif
    if(success){
        ofLogNotice("ofxSimpleApp::saveXmlSettings()") << "Saved settings to `" << path << "`.";
        return true;
    }
    else {
        ofLogError("ofxSimpleApp::saveXmlSettings()") << "Failed saving settings to `" << path << "` !";
        return false;
    }
}

// Populate XML for saving
#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
bool ofxSimpleApp::ofxSA_populateXmlSettings(pugi::xml_node& _node){
    bool ret = true;

    // Version
    _node.append_attribute("version_maj").set_value(ofxSA_APP_VERSION_MAJOR);
    _node.append_attribute("version_min").set_value(ofxSA_APP_VERSION_MINOR);
    _node.append_attribute("version_patch").set_value(ofxSA_APP_VERSION_PATCH);

    // App name, just for info
    _node.append_child("app_name").text().set(ofxSA_APP_NAME);

    // Theme
    _node.append_child("gui_theme").text().set(themeID);

    // Logging
    _node.append_child("show_log_window").text().set(bShowLogs?true:false);
    _node.append_child("log_level").text().set(ofGetLogLevel());

    // Document settings
    pugi::xml_node docNode = _node.append_child("document");
#ifdef ofxSA_BACKGROUND_CLEARING
	ofxPugiXml::setNodeValueToAttribute(docNode, "doc_clearing", bEnableClearing);
	ofxPugiXml::setNodeValueToAttribute(docNode, "doc_clearing_color", bgClearColor);
#endif
#ifdef ofxSA_BACKGROUND_FADING
	ofxPugiXml::setNodeValueToAttribute(docNode, "doc_fading", bEnableFading);
	ofxPugiXml::setNodeValueToAttribute(docNode, "doc_fading_color", bgFadeColor);
#endif

    // todo: targetFPS, resolution, fullscreen, etc.

    // Canvas
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    pugi::xml_node canvasSettingsNode = _node.append_child("canvas");
    ret *= canvasSettingsNode && canvas.populateXmlNode(canvasSettingsNode);
#endif

    // Timeline settings
#ifdef ofxSA_TIMELINE_ENABLE
    pugi::xml_node timelineSettingsNode = _node.append_child("timeline");
    ofxPlayhead& tl = ofxSA_TIMELINE_GET(timeline);
    ret *= timelineSettingsNode && tl.populateXmlNode(timelineSettingsNode);
#endif

    // todo: Recording settings
#ifdef ofxSA_TEXRECORDER_ENABLE
    pugi::xml_node texRecorderSettingsNode = _node.append_child("texture_recorder");
    texRecorderSettingsNode.append_child("recorder_mode").text().set(texRecorderMode);
#endif

    // Done
    return ret;
}

// Retrieve data from XML for saving
bool ofxSimpleApp::ofxSA_retrieveXmlSettings(pugi::xml_node& _node){
    bool ret = true;

    unsigned int vMaj = _node.attribute("version_maj").as_uint();
    unsigned int vMin = _node.attribute("version_min").as_uint();
    unsigned int vPatch = _node.attribute("version_patch").as_uint();
    const char* appName = _node.child("app_name").child_value();

    // Check app name
    if( strcmp(appName, ofxSA_APP_NAME) != 0 ){
        ofLogWarning("ofxSimpleApp::ofxSA_retrieveXmlSettings") << "This save file was not made for the same ofxSimpleApp ("<< appName <<"instead of "<< ofxSA_APP_NAME <<"). Proceeding anyways but proceed with caution !";
        // Todo: increment save file, load new one not to overwrite old version ?
    }

    // Check app version
    if(ofxSA_APP_VERSION_MAJOR != vMaj || ofxSA_APP_VERSION_MINOR != vMin || ofxSA_APP_VERSION_PATCH != vPatch){
        ofLogWarning("ofxSimpleApp::ofxSA_retrieveXmlSettings") << "Loaded XML file was made using an older version ("<<vMaj<<"."<<vMin<<"."<<vPatch<<"). Runtime version = "<<ofxSA_APP_VERSION_MAJOR<<"."<<ofxSA_APP_VERSION_MINOR<<"."<<ofxSA_APP_VERSION_PATCH<<".";
        // Todo: increment save file, load new one not to overwrite old version ?
    }

    // Grab theme
    themeID = _node.child("gui_theme").text().as_int(ofxSA_GUI_THEME_DEFAULT);
    loadImGuiTheme();

    // Logging window visibility
    bShowLogs = _node.child("show_log_window").text().as_bool(false);

    // Log level
    if(pugi::xml_node lvlChild = _node.child("log_level")){
        ofLogLevel lvl = static_cast<ofLogLevel>(lvlChild.text().as_int((int)OF_LOG_NOTICE));
        ofSetLogLevel(lvl);
    }

        // Document settings
    pugi::xml_node docNode = _node.append_child("document");
#ifdef ofxSA_BACKGROUND_CLEARING
	ofxPugiXml::getNodeValueFromAttribute(docNode, "doc_clearing", bEnableClearing);
	ofxPugiXml::getNodeValueFromAttribute(docNode, "doc_clearing_color", bgClearColor);
#endif
#ifdef ofxSA_BACKGROUND_FADING
	ofxPugiXml::getNodeValueFromAttribute(docNode, "doc_fading", bEnableFading);
	ofxPugiXml::getNodeValueFromAttribute(docNode, "doc_fading_color", bgFadeColor);
#endif

    // Canvas
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    pugi::xml_node canvasSettingsNode = _node.child("canvas");
    ret *= canvasSettingsNode && canvas.retrieveXmlNode(canvasSettingsNode);
#endif

    // Todo: Timeline settings
#ifdef ofxSA_TIMELINE_ENABLE
    ofxPlayhead& tl = ofxSA_TIMELINE_GET(timeline);
    pugi::xml_node timelineSettingsNode = _node.child("timeline");
    ret *= timelineSettingsNode && tl.retrieveXmlNode(timelineSettingsNode);
#endif

    // todo: Recording settings
#ifdef ofxSA_TEXRECORDER_ENABLE
    pugi::xml_node texRecorderSettingsNode = _node.child("texture_recorder");
    if(pugi::xml_node modeChild = texRecorderSettingsNode.child("recorder_mode")){
        texRecorderMode = static_cast<TexRecorderMode_>(modeChild.text().as_int((int)TexRecorderMode_PNG));
    }
#endif

    return ret;
}
#endif // ofxSA_XML_ENGINE_PUGIXML

#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
unsigned int ofxSimpleApp::getCanvasResolutionX() const {
    return canvas.getCanvasResolutionX();
}
unsigned int ofxSimpleApp::getCanvasResolutionY() const {
    return canvas.getCanvasResolutionY();
}
void ofxSimpleApp::onCanvasViewportResize(ofRectangle& args){
    onViewportChange();
}
void ofxSimpleApp::onCanvasContentResize(ContentResizeArgs& _args){
    onContentResize(_args.width*_args.scale, _args.height*_args.scale);
}
#endif

#ifdef ofxSA_TEXRECORDER_ENABLE
#	ifdef ofxSA_TIMELINE_ENABLE
bool ofxSimpleApp::onTimelineRestart(const std::size_t& _loopCount){
	if(isRecordingCanvas && bRecorderStopOnLoop && _loopCount>0){
		stopRecordingCanvas();
	}
	return true;
}
bool ofxSimpleApp::onTimelineStop(){
	if(isRecordingCanvas){
		stopRecordingCanvas();
	}
	return true;
}
#	endif
#endif

#ifdef ofxSA_NDI_SENDER_ENABLE
bool ofxSimpleApp::startNdi(){
#   ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    const bool ndiCreated = ndiSender.CreateSender(ofxSA_APP_NAME, canvas.getCanvasWidth(), canvas.getCanvasHeight());
#   else
    const bool ndiCreated = ndiSender.CreateSender(ofxSA_APP_NAME, ofGetWindowWidth(), ofGetWindowHeight());
#   endif
    if(!ndiCreated){
        ofLogWarning("ofxSimpleApp::Setup()") << "Couldn't create NDI server !";
    }
    return ndiCreated;
}

void ofxSimpleApp::stopNdi(){
    ndiSender.ReleaseSender();
}
#endif

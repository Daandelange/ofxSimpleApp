#include "ofxSimpleApp.h"
//#include "imgui_internal.h"
#include "imgui.h"

#include "Spectrum.h"
#include "DarkTheme.h"

#include <string>

#if ofxSA_GUI_THEME != ofxSA_GUI_THEME_DARK && ofxSA_GUI_THEME != ofxSA_GUI_THEME_LIGHT
#	include ofxSA_GUI_THEME_FILE
#endif

#include "ofxSimpleAppUtils.h"
#include "imgui_internal.h" // <-- advanced docking features from imgui internals...
//#include "ofGraphics.h" // for GL info stuff ?

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

bool ofxSimpleApp::bUseDarkTheme = true;
ofxImGui::BaseTheme* ofxSimpleApp::imguiTheme = nullptr;
const int ofxSimpleApp:: curYear = std::stoi(ofGetTimestampString("%Y").c_str());

//--------------------------------------------------------------
ofxSimpleApp::ofxSimpleApp(){
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
	//ofGetGLRenderer()->
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

	// Window Title
	// Done in main.cpp, useless here
	//ofSetWindowTitle(ofxSA_APP_NAME);

	// - - - - - -
	// GUI SETUP

	// Gui Instance
	gui.setup(bUseDarkTheme?((ofxImGui::BaseTheme*)new DarkTheme()):((ofxImGui::BaseTheme*)new Spectrum()), false, ofxSA_UI_IMGUI_CONFIGFLAGS, ofxSA_UI_RESTORE_STATE);

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
	canvas.setScreenRect(); // todo: make this use viewports ?
	ofAddListener(canvas.onViewportResize, this, &ofxSimpleApp::onCanvasViewportResize);
	ofAddListener(canvas.onContentResize, this, &ofxSimpleApp::onCanvasContentResize);
#endif

#ifdef ofxSA_TIMELINE_ENABLE
#	if ofxSA_TIMELINE_AUTOSTART == true
	ofxSA_TIMELINE_GET(timeline).start();
#	endif
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
	m_Recorder.setFFmpegPath(ofToDataPath("ffmpeg/osx/ffmpeg"));
    m_Recorder.setFFmpegPath(ofToDataPath("C:/dev/msys64/ThirdParty/ffmpeg/ffmpeg_build/bin/ffmpeg.exe", true));
    #endif

    // Set default codec
    m_Recorder.setVideoCodec(ofxSA_TEXRECORDER_DEFAULT_CODEC);
    
    // Configure FBO reader
    //fastFboReader.setAsync(false);
#endif

	// Restore settings on launch
	loadXmlSettings();

}

void ofxSimpleApp::exit(){

}

//--------------------------------------------------------------
void ofxSimpleApp::update(){
    // FPS plotting
    syncHistogram<float, double>(FPSHistory, ofGetFrameRate());

#ifdef ofxSA_TIMELINE_ENABLE
    ofxSA_TIMELINE_GET(timeline).tickUpdate();
#endif
}

//--------------------------------------------------------------
void ofxSimpleApp::draw(){
#ifdef ofxSA_TIMELINE_ENABLE
    ofxSA_TIMELINE_GET(timeline).tickFrame();
#endif

#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    canvas.draw();
#endif

#ifdef ofxSA_SYPHON_OUTPUT
    publishSyphonTexture();
#endif

#ifdef ofxSA_TEXRECORDER_ENABLE
    recordCanvasFrame();
#endif

    renderGui();
}

//--------------------------------------------------------------
#ifdef ofxSA_SYPHON_OUTPUT
void ofxSimpleApp::publishSyphonTexture(){
    // By default, this publishes the full window.
    // You can override this to publish a custom texture.
    syphonServer.publishScreen();
}
#endif

//--------------------------------------------------------------
#ifdef ofxSA_TEXRECORDER_ENABLE
bool ofxSimpleApp::startRecordingCanvas(){
    // Set new target file
#   if defined(TARGET_OSX)
    m_Recorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".mp4", true ));
#   else
    m_Recorder.setOutputPath( ofToDataPath(ofGetTimestampString() + ".avi", true ));
#   endif

    // Recording settings
    m_Recorder.setVideoCodec(ofxSA_TEXRECORDER_DEFAULT_CODEC);
    m_Recorder.setInputPixelFormat("rgba");
    // example test with h264_nvenc
    //m_Recorder.setVideoCodec("h264_nvenc");
    m_Recorder.setBitRate(8000);

    isRecordingCanvas = m_Recorder.startCustomRecord();

    return isRecordingCanvas;
}
bool ofxSimpleApp::stopRecordingCanvas(){
    m_Recorder.stop();
    isRecordingCanvas = false;

    return isRecordingCanvas == false;
}
void ofxSimpleApp::recordCanvasFrame(){
    //static ofPixels mPix;
    if (m_Recorder.isRecording()) {
		if (isRecordingCanvas && canvas.fbo.isAllocated()) {
            // Read FBO to pixels
            //canvas.fbo.updateTexture(0);
			canvas.fbo.readToPixels(recordedPixels);
            fastFboReader.setAsync(false);
            if(true){//} fastFboReader.readToPixels(canvas.fbo, recordedPixels, OF_IMAGE_COLOR_ALPHA)){
            //if(fastFboReader.readToPixels(canvas.fbo, recordedPixels, GL_RGBA32F)){
                ofSaveImage(recordedPixels, ofToDataPath("testScreenShot.png"));

                // Add frame
                if (recordedPixels.getWidth() > 0 && recordedPixels.getHeight() > 0) {
                    m_Recorder.addFrame(recordedPixels);
                }
                
            }
            else {
                std::cout << "failed readToPixels !" << std::endl;
                //canvas.fbo.readToPixels(mPix); // fallback on native pixels grabbing
            }
            
		}
	}
}
#endif

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
        gui.begin();
        drawGui();

        // Force metrics at the end of ImGui submissions
        if(bShowImGuiDemo)          ImGui::ShowDemoWindow(&bShowImGuiDemo);
        if(bShowImGuiDebugWindow)   ImGui::ShowDebugLogWindow(&bShowImGuiDebugWindow);
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
    // Son't interfecre with ImGui
    if(ImGui::GetIO().WantCaptureKeyboard) return;

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
    }
}

////--------------------------------------------------------------
void ofxSimpleApp::keyReleased(ofKeyEventArgs &e){

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
    if(imguiTheme!=nullptr) delete imguiTheme;
    //if(bUseDarkTheme) imguiTheme = new DarkTheme();
    //else imguiTheme = new Spectrum();
    //imguiTheme->setup();
    gui.setTheme(bUseDarkTheme?((ofxImGui::BaseTheme*)new DarkTheme()):((ofxImGui::BaseTheme*)new Spectrum()));
}

void ofxSimpleApp::ImGuiDrawMenuBar(){
    if( ImGui::BeginMainMenuBar() ){
        if(ImGui::BeginMenu( ofxSA_APP_NAME )){
            // Info
            if(ofxSA_APP_COPYRIGHT_START_YEAR != 0 && ofxSA_APP_COPYRIGHT_START_YEAR != curYear){
                ImGui::Text("Copyright %i-%i %s", ofxSA_APP_COPYRIGHT_START_YEAR, curYear, ofxSA_APP_AUTHOR);
            }
            else {
                ImGui::Text("Copyright %i %s", curYear, ofxSA_APP_AUTHOR);
            }
            ImGui::Text("Version %d.%d.%d", ofxSA_VERSION_MAJOR, ofxSA_VERSION_MINOR, ofxSA_VERSION_PATCH );
            if(ImGui::MenuItem("About")){
                bShowAboutWindow = !bShowAboutWindow;
            }

            // Logging
            ImGui::Dummy({5,5});
            ImGui::SeparatorText("Logging");
            ImGui::Checkbox("Show log window", &bShowLogs);
            ofLogLevel curLogLevel = ofGetLogLevel();
            static std::pair<ofLogLevel, std::string> ofLogLevels[] = {
                { OF_LOG_VERBOSE,       ofGetLogLevelName(OF_LOG_VERBOSE    ) },
                { OF_LOG_NOTICE,        ofGetLogLevelName(OF_LOG_NOTICE     ) },
                { OF_LOG_WARNING,       ofGetLogLevelName(OF_LOG_WARNING    ) },
                { OF_LOG_ERROR,         ofGetLogLevelName(OF_LOG_ERROR      ) },
                { OF_LOG_FATAL_ERROR,   ofGetLogLevelName(OF_LOG_FATAL_ERROR) },
                { OF_LOG_SILENT,        ofGetLogLevelName(OF_LOG_SILENT     ) }
            };
            if( ImGui::BeginCombo("Log level", ofGetLogLevelName(curLogLevel).c_str()) ){
                for(const auto& logLevel : ofLogLevels){
                    if(ImGui::Selectable(logLevel.second.c_str(), curLogLevel == logLevel.first ) ){
                        ofSetLogLevel(logLevel.first);
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Dummy({5,5});
            ImGui::SeparatorText("Gui");
            //ImGui::Checkbox("Show Gui", &bShowGui);
            if(ImGui::MenuItem("Hide GUI", SHORTCUT_FUNC "+G")){
                bShowGui = false;
                updateCursorForGui();
            }

            if(ImGui::Checkbox("Use dark theme", &bUseDarkTheme)){
                loadImGuiTheme();
            }
            ImGui::Separator();
            //ImGui::Dummy({ofxSA_UI_MARGIN,ofxSA_UI_MARGIN});
            if(ImGui::MenuItem("Toggle full screen", SHORTCUT_FUNC "+F")){
                ofToggleFullscreen();
                onImguiViewportChange();
            }

            ImGui::Dummy({5,5});
            ImGui::SeparatorText("Document");
            ImGui::TextDisabled("%s", savePath.c_str());
            ImGui::SameLine();
            ImGui::Text("%s", saveName.c_str());
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
                        if(ImGui::Selectable(file.getFileName().c_str(), false, ImGuiSelectableFlags_DontClosePopups)){
                            strncpy(newFileName, "\0", IM_ARRAYSIZE(newFileName)-1);
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

            ImGui::Dummy({5,5});
            ImGui::Separator();
            if( ImGui::MenuItem( "Exit", SHORTCUT_FUNC "+Q" ) ){
                ofExit();
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

        if(ImGui::BeginMenu("Modules")){
            ImGui::SeparatorText("ofxSimpleApp");
#ifdef ofxSA_DEBUG
            ImGui::Checkbox("Show ImGui Metrics", &bShowImGuiMetrics);
            ImGui::Checkbox("Show ImGui Debug Window", &bShowImGuiDebugWindow);
            ImGui::Checkbox("Show ImGui Demo Window", &bShowImGuiDemo);
#endif

#ifdef ofxSA_SYPHON_OUTPUT
            ImGui::SeparatorText("Syphon");
            ImGui::Checkbox("Enable syphon output", &bEnableSyphonOutput);
            ImGui::BeginDisabled(true);
            ImGui::Text("Server name: %s", syphonServer.getName().c_str());
            ImGui::EndDisabled();
#endif

#ifdef ofxSA_TEXRECORDER_ENABLE
            ImGui::SeparatorText("Texture Recorder");
            
            ImGui::BeginDisabled(true);
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
            ImGui::Text("FFmpeg   : %s", m_Recorder.getFFmpegPath().c_str());
            ImGui::Text("Bitrate  : %u", m_Recorder.getBitRate());
            ImGui::Text("FPS      : %.2f", m_Recorder.getFps());
            ImGui::Text("Width    : %.0f", m_Recorder.getWidth());
            ImGui::Text("Height   : %.0f", m_Recorder.getHeight());
            ImGui::Text("Duration : %.2f / %.2f", m_Recorder.getCaptureDuration(), m_Recorder.getRecordedDuration());
            ImGui::Text("File     : %s", m_Recorder.getOutputPath().c_str());
            ImGui::Text("Codec    : %s", m_Recorder.getVideoCodec().c_str());
#endif

            ImGui::EndMenu();
        }

        // View menu
        if(ImGui::BeginMenu("View")){
            ImGui::Checkbox("Timeline", &bShowTimeClockWindow);
            ImGui::Separator();
            ImGui::EndMenu();
        }

        // Spacer for custom menu items (after)
        ImGui::Text("\t");
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

            ImGui::Text("Version %d.%d.%d", ofxSA_VERSION_MAJOR, ofxSA_VERSION_MINOR, ofxSA_VERSION_PATCH );
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
                    ImGui::BeginChildFrame(ImGui::GetID("Build Configuration"), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 18), ImGuiWindowFlags_NoMove);
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
                    ImGui::EndChildFrame();

                    ImGui::EndTabItem();
                } // end build info

                // Runtime
                if( false && ImGui::BeginTabItem("Runtime") ){
                    // osx
                    // Required to install cude devkit : https://developer.nvidia.com/cuda-downloads
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

    dockingFlags |= ImGuiDockNodeFlags_NoDockingInCentralNode; // Uncomment to always keep an empty "central node" (a visible oF space)
    //dockingFlags |= ImGuiDockNodeFlags_NoTabBar; // Uncomment to disable creating tabs in the main view
    //dockingFlags |= ImGuiDockNodeFlags_AutoHideTabBar;
    //dockingFlags |= ImGuiDockNodeFlags_NoDockingSplitMe;
//    dockingFlags |= ImGuiDockNodeFlags_NoDockingOverMe;

    // Define the ofWindow as a docking space
    ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(NULL, dockingFlags); // Also draws the docked windows
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

            ImVec2 viewCenter = availableSpace.GetCenter();
            // Depending on the viewports flag, the XY is either absolute or relative to the oF window.
            if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) viewCenter = viewCenter - ImVec2(ofGetWindowPositionX(),ofGetWindowPositionY());

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
#endif

#ifdef ofxSA_TIMELINE_ENABLE
void ofxSimpleApp::ImGuiDrawTimeline(){
    if(bShowTimeClockWindow){
        ofxSA_TIMELINE_GET(timeline).drawImGuiTimelineWindow(&bShowTimeClockWindow);
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

ofRectangle ofxSimpleApp::getViewport() const {
    // Set coords to window coords
    ofRectangle curViewport(0,0, ofGetWidth(), ofGetHeight());

//    curViewport.x = 0;
//    curViewport.y = 0;
//    curViewport.width = ofGetWidth();
//    curViewport.height = ofGetHeight();

    // Subtract gui zones
    if(bShowGui){

#if ofxSA_UI_DOCKING_ENABLE_DOCKSPACES == 1
        // Return visible zone (without docking spaces and menus)
        return dockingViewport;
#else
        // Subtract Menu bar
        float menuHeight = ImGui::GetFrameHeight();
        curViewport.x += menuHeight;
        curViewport.height -= menuHeight;
#endif

    }

    return curViewport;
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
#ifdef ofxSA_XML_ENGINE_PUGIXML
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
#ifdef ofxSA_XML_ENGINE_PUGIXML
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
    pugi::xml_node customAppSettingsNode = doc.child(ofxSA_APP_NAME);
    if(!customAppSettingsNode){
        ret = false;
        ofLogWarning("ofxSimpleApp::loadXmlSettings()") << "There's no `" << ofxSA_APP_NAME << "` section in the document !";
    }
    else {
        if(retrieveXmlSettings(customAppSettingsNode)){
            ofLogVerbose("ofxSimpleApp::loadXmlSettings()") << "Imported `" << ofxSA_APP_NAME << "` section from the document !";
        }
        else {
            ret = false;
            ofLogWarning("ofxSimpleApp::loadXmlSettings()") << "Couldn't parse `" << ofxSA_APP_NAME << "` section in the document !";
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
#ifdef ofxSA_XML_ENGINE_PUGIXML
    pugi::xml_document doc;
    pugi::xml_node ofxSimpleAppSettingsNode = doc.append_child("ofxSimpleAppSettings");
    success *= ofxSimpleApp::ofxSA_populateXmlSettings(ofxSimpleAppSettingsNode);
//    ofxSimpleAppSettingsNode.append_attribute("version_maj").set_value(ofxSA_VERSION_MAJOR);
//    ofxSimpleAppSettingsNode.append_attribute("version_min").set_value(ofxSA_VERSION_MINOR);
//    ofxSimpleAppSettingsNode.append_attribute("version_patch").set_value(ofxSA_VERSION_PATCH);
//    ofxSimpleAppSettingsNode.append_child("app_name").append_child(pugi::node_pcdata).set_value(ofxSA_APP_NAME);

    pugi::xml_node customAppSettingsNode = doc.append_child(ofxSA_APP_NAME);
    success *= populateXmlSettings(customAppSettingsNode);

    if(success){
        // Save !
        doc.save_file(ofToDataPath(path).c_str());
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
        ofLogNotice("ofxSimpleApp::saveXmlSettings()") << "Saved settings to `settings.xml` !";
        return true;
    }
    else {
        ofLogError("ofxSimpleApp::saveXmlSettings()") << "Failed saving settings to `settings.xml` !";
        return false;
    }
}

// Populate XML for saving
bool ofxSimpleApp::ofxSA_populateXmlSettings(pugi::xml_node& _node){
    // Version
    _node.append_attribute("version_maj").set_value(ofxSA_VERSION_MAJOR);
    _node.append_attribute("version_min").set_value(ofxSA_VERSION_MINOR);
    _node.append_attribute("version_patch").set_value(ofxSA_VERSION_PATCH);

    // App name, just for info
    _node.append_child("app_name").append_child(pugi::node_pcdata).set_value(ofxSA_APP_NAME);

    // Theme
    // todo: support custom themes
    _node.append_child("gui_theme").append_child(pugi::node_pcdata).set_value(std::to_string(bUseDarkTheme?ofxSA_GUI_THEME_DARK:ofxSA_GUI_THEME_LIGHT).c_str());

    // todo: targetFPS, resolution, fullscreen, etc.

    // Done
    return true;
}

// Retrieve data from XML for saving
bool ofxSimpleApp::ofxSA_retrieveXmlSettings(pugi::xml_node& _node){
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
    if(ofxSA_VERSION_MAJOR != vMaj || ofxSA_VERSION_MINOR != vMin || ofxSA_VERSION_PATCH != vPatch){
        ofLogWarning("ofxSimpleApp::ofxSA_retrieveXmlSettings") << "Loaded XML file was made using an older version ("<<vMaj<<"."<<vMin<<"."<<vPatch<<"). Runtime version = "<<ofxSA_VERSION_MAJOR<<"."<<ofxSA_VERSION_MINOR<<"."<<ofxSA_VERSION_PATCH<<".";
        // Todo: increment save file, load new one not to overwrite old version ?
    }

    // Grab theme
    const char* theme = _node.child("gui_theme").child_value();
    if(theme && theme[0] != '\0'){
        if(ofxSA_GUI_THEME_DARK == std::stoi(theme)) bUseDarkTheme = true;
        else if(ofxSA_GUI_THEME_LIGHT == std::stoi(theme)) bUseDarkTheme = false;
        //else if(ofxSA_GUI_THEME_CUSTOM = theme) bUseDarkTheme = true;
    }

    return true;
}

#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
unsigned int ofxSimpleApp::getCanvasResolutionX() const {
    return canvas.getCanvasResolutionX();
}
unsigned int ofxSimpleApp::getCanvasResolutiony() const {
    return canvas.getCanvasResolutionY();
}
void ofxSimpleApp::onCanvasViewportResize(ofRectangle& args){
    onViewportChange();
}
void ofxSimpleApp::onCanvasContentResize(ContentResizeArgs& _args){
    onContentResize(_args.width*_args.scale, _args.height*_args.scale);
}
#endif

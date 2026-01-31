#include "ofxSimpleAppCanvas.h"
#include "GLFW/glfw3.h"
#include "ofxPugiXMLHelpers.h"

template<typename TYPE, int SIZE>
const char* getItemFromPairArray(TYPE _mode, const std::pair<TYPE, const char*>(&map)[SIZE] ) {
    for(const auto& mode : map){
        if(mode.first==_mode) return mode.second;
    }
    return "unknown";
}

const std::pair<GlSampleMode_, const char*> glSampleModes[2] = {
    { GlSampleMode_Linear,          "Linear" },
    { GlSampleMode_NearestNeighbour,"Nearest" },
};

const char* glSampleModeGetName(GlSampleMode_ _mode ) {
    return getItemFromPairArray(_mode, glSampleModes);
    for(const auto& mode : glSampleModes){
        if(mode.first==_mode) return mode.second;
    }
    return "unknown";
    //switch(_mode){
    //    case GlSampleMode_Linear:
    //        return "Linear";
    //        break;
    //    case GlSampleMode_NearestNeighbour:
    //        return "Nearest";
    //        break;
    //}
}
const std::pair<GlTexRepeatMode_, const char*> glTexRepeatModes[4] = {
    { GlTexRepeatMode_Repeat,           "Tile" },
    { GlTexRepeatMode_RepeatMirrored,   "Mirrored" },
    { GlTexRepeatMode_ClampEdge,        "Edge" },
    { GlTexRepeatMode_BorderColor,      "Static" },
};

const char* glTexRepeatModeGetName(GlTexRepeatMode_ _mode ) {
    return getItemFromPairArray(_mode, glTexRepeatModes);
    for(const auto& mode : glTexRepeatModes){
        if(mode.first==_mode) return mode.second;
    }
    return "unknown";
}

namespace ImGuiEx {
    void glSampleModeImGuiCombo(const char* label, GlSampleMode_& value){
        if( ImGui::BeginCombo(label, glSampleModeGetName(value)) ){
            for(const auto& mode : glSampleModes){
                if(ImGui::Selectable(mode.second, value == mode.first ) ){
                    value = mode.first;
                }
            }
            ImGui::EndCombo();
        }
    }

    void glTexRepeatModeImGuiCombo(const char* label, GlTexRepeatMode_& value){
        if( ImGui::BeginCombo(label, glTexRepeatModeGetName(value)) ){
            for(const auto& mode : glTexRepeatModes){
                if(ImGui::Selectable(mode.second, value == mode.first ) ){
                    value = mode.first;
                }
            }
            ImGui::EndCombo();
        }
    }
}

ContentResizeArgs::ContentResizeArgs(unsigned int _width, unsigned int _height, float _scale) :
    width(_width),
    height(_height),
    scale(_scale)
{

}

ofxSimpleAppCanvas::ofxSimpleAppCanvas(unsigned int _width, unsigned int _height){
    setViewportRect();
    setCanvasSize(_width, _height);

    // Auto-create standalone window ?
#if ofxSA_CANVAS_OUTPUT_EXTRA_STANDALONE_WINDOW == 1
    bRenderExtraCanvasWindow = false;
#   if ofxSA_SECONDARY_WINDOW_AUTOENABLED
    if(auto mainWin = ofGetCurrentWindow()){ // Fixme: this is supposed to be the main window, not ensured.
        if(auto mainWinGlfw = std::static_pointer_cast<ofAppGLFWWindow>(mainWin)){
            setupSecondaryMonitor(mainWinGlfw);
        }
    }
#   endif
#endif
}

#if ofxSA_CANVAS_OUTPUT_EXTRA_STANDALONE_WINDOW == 1
void ofxSimpleAppCanvas::onSecondaryWindowClose(ofEventArgs& args){
    extraCanvasOutputWindowPtr = nullptr; // forget instance = autodestruct. Prevents calling methods on unvalid glfw handle.
}
void ofxSimpleAppCanvas::drawSecondaryMonitor(ofEventArgs &args){
    // Always clear (black screen if not ready to draw)
    ofBackground(0);

    // Draw image ?
    if(extraCanvasOutputWindowPtr && bRenderExtraCanvasWindow && fbo.isAllocated()){
        fbo.draw(0,0);
    }

}
void ofxSimpleAppCanvas::closeSecondaryWindow(){
    if(extraCanvasOutputWindowPtr){
        extraCanvasOutputWindowPtr->setWindowShouldClose();
        extraCanvasOutputWindowPtr = nullptr;
    }
}

void ofxSimpleAppCanvas::setupSecondaryMonitor(std::shared_ptr<ofAppGLFWWindow> sharedGlfwWindow){
    if(!sharedGlfwWindow || !sharedGlfwWindow->getGLFWWindow()){
        ofLogError("ofxSimpleAppCanvas::setupSecondaryMonitor") << "The window is not correctly setup and cannot be used !";
        return;
    }
    ofGLFWWindowSettings outputWinSettings;
    outputWinSettings.setGLVersion(ofxSA_GL_VERSION_MAJ, ofxSA_GL_VERSION_MIN); // important for shareContextWith: share within same GL version
    outputWinSettings.title = "Canvas Output";
    //settings.monitor = 0; // todo: set to 2nd monitor ?
    if(sharedGlfwWindow){
        outputWinSettings.shareContextWith = sharedGlfwWindow;
    }
    else {
        ofLogWarning("ofxSimpleAppCanvas::setupSecondaryMonitor") << "Shared GL context window is not set ! The window will probably not work correctly !";
    }
    //outputWinSettings.windowMode=OF_GAME_MODE;
    outputWinSettings.setSize(this->getCanvasWidth(), this->getCanvasHeight());
    outputWinSettings.setPosition(ofVec2f(0,0));

    // Create window
#   if 0
    // Like: ofCreateWindow, but keeping the detailed ptr
    ofInit();
    extraCanvasOutputWindowPtrGlfw = std::make_shared<ofAppGLFWWindow>();//ofCreateWindow();
    extraCanvasOutputWindowPtrBase = extraCanvasOutputWindowPtrGlfw;
    ofGetMainLoop()->addWindow(extraCanvasOutputWindowPtrGlfw);
    extraCanvasOutputWindowPtrGlfw->setup(settings);
#   else
    auto extraCanvasOutputWindowPtrBase = ofCreateWindow(outputWinSettings);//ofGetMainLoop()->createWindow(settings);
    extraCanvasOutputWindowPtr = std::static_pointer_cast<ofAppGLFWWindow>(extraCanvasOutputWindowPtrBase);
#       ifdef ofxSA_DEBUG
    const char* description;
    int code = glfwGetError(&description);
    if (description)
        std::cout << ("ofxSimpleAppCanvas::setupSecondaryMonitor") << "Glfw couldn't create window ! Code="<< code << ", Description="<< description << std::endl;
    else if(extraCanvasOutputWindowPtr->getGLFWWindow() == nullptr)
        std::cout << ("ofxSimpleAppCanvas::setupSecondaryMonitor") << "No GLFW error but still incorrectly created ! (glfw)" << std::endl;
#       endif

    // Note: Reset of to main window, if events register against ofGetCurrentWindow()
    if(sharedGlfwWindow) ofGetMainLoop()->setCurrentWindow(sharedGlfwWindow);
#   endif
#   ifdef ofxSA_DEBUG
    std::cout << "Created 2ndary window ! @" << extraCanvasOutputWindowPtr->getGLFWWindow() << "/sharedContext=" << outputWinSettings.shareContextWith << extraCanvasOutputWindowPtr->getWidth() << "x" << extraCanvasOutputWindowPtr->getHeight() << std::endl;
#   endif

    if(extraCanvasOutputWindowPtr){
        ofAddListener(extraCanvasOutputWindowPtr->events().exit, this, &ofxSimpleAppCanvas::onSecondaryWindowClose);
        ofAddListener(extraCanvasOutputWindowPtr->events().draw, this, &ofxSimpleAppCanvas::drawSecondaryMonitor);
    }
}
#endif

void ofxSimpleAppCanvas::setCanvasSize(unsigned int _width, unsigned int _height, float _scale){
    height = _height;
    width = _width;
    scale = _scale;
    fbo.allocate(width*scale, height*scale, GL_RGBA);
    bFlagDirty = true;

    ContentResizeArgs eventArgs = ContentResizeArgs(width, height, scale);
    ofNotifyEvent(onContentResize, eventArgs, this);
}

// (Unscaled)
glm::vec2 ofxSimpleAppCanvas::getCanvasSize() const {
    return glm::vec2(width, height);
}
unsigned int ofxSimpleAppCanvas::getCanvasWidth() const {
    return width;
}
unsigned int ofxSimpleAppCanvas::getCanvasHeight() const {
    return height;
}

glm::vec2 ofxSimpleAppCanvas::getCanvasResolution() const {
    return glm::vec2(getCanvasResolutionX(), getCanvasResolutionY());
}

unsigned int ofxSimpleAppCanvas::getCanvasResolutionX() const {
    return fbo.isAllocated()?fbo.getWidth():0;
}

unsigned int ofxSimpleAppCanvas::getCanvasResolutionY() const {
    return fbo.isAllocated()?fbo.getHeight():0;
}

// Returns current zoom level (in UI)
float ofxSimpleAppCanvas::getViewZoom() const {
    ofRectangle vp = viewportRect;
    //glm::vec2 contentSize = getCanvasResolution();
    glm::vec2 contentSize = getCanvasSize();

    float _viewZoom = viewZoom;
    // Adapt zoom to auto modes
    if(contentDrawMode & CanvasDrawMode_AutoCover){
        _viewZoom = glm::max(vp.width/contentSize.x, vp.height/contentSize.y);
    }
    if(contentDrawMode & CanvasDrawMode_AutoContain){
        _viewZoom = glm::min(vp.width/contentSize.x, vp.height/contentSize.y);
    }

    if(_viewZoom==0) _viewZoom = 1; // to never divide by 0 !

    return _viewZoom;
}

// Renderer scale (aka antialiasing)
float ofxSimpleAppCanvas::getScale() const {
    return scale;
}

glm::vec2 ofxSimpleAppCanvas::getViewTranslation() const {

    glm::vec2 translation = viewTranslation;//canvas.getViewTranslation();

    // Zoom the translation too
    if(contentDrawMode & CanvasDrawMode_AutoCover || contentDrawMode & CanvasDrawMode_AutoContain){
        // Center imaage by default in auto modes
        translation.x /= viewZoom;
        translation.y /= viewZoom;
    }

    // Center content in middle of viewport
    glm::vec2 contentSize = getCanvasResolution();
    translation.x += (contentSize.x-width*scale)*.5f;
    translation.y += (contentSize.y-height*scale)*.5f;

    return viewTranslation;
}

// Sets the viewport position in the ofAppWindow
void ofxSimpleAppCanvas::setViewportRect(unsigned int _width, unsigned int _height, int _x, int _y){
    viewportRect.x=_x;
    viewportRect.y=_y;
    viewportRect.width=_width;
    viewportRect.height=_height;
    bFlagRepaint = true;

    //ofRectangle eventArgs = ofRectangle(width, height); in screen coords
    ofNotifyEvent(onViewportResize, viewportRect, this);
}

ofRectangle ofxSimpleAppCanvas::getViewportRect() const {
    return viewportRect;
}

ofRectangle ofxSimpleAppCanvas::getContentProjection() const {
    ofRectangle vp = getViewportRect();
    glm::vec2 contentSize = getCanvasResolution();
    float viewZoom = getViewZoom();

    ofRectangle textureArea = {0,0,vp.width*scale, vp.height*scale};

    // Scale
    textureArea.width /= viewZoom;
    textureArea.height /= viewZoom;

    glm::vec2 translation = getViewTranslation();

    // Translate
    textureArea.x += translation.x;// + (contentSize.x-textureArea.width)*.5f;
    textureArea.y += translation.y;// + (contentSize.y-textureArea.height)*.5f;

    // Re-center view if out of bounds
//        if(contentSize.x < textureArea.width && textureArea.x+textureArea.width+allowedCanvasBorder > contentSize.x){
//            textureArea.x = contentSize.x - textureArea.width + allowedCanvasBorder;
//        }

    return textureArea;
}

// Draws the FBO content to screen and an optional viewport rect.
void ofxSimpleAppCanvas::draw(){
    ofPushStyle();
    if(fbo.isAllocated()){
        //ofFill();
        //ofSetColor(ofColor::white);
        //fbo.draw(0,0);//viewportRect.x, viewportRect.y, viewportRect.width, viewportRect.height);

        ofRectangle cvp = getViewportRect();
        ofRectangle ctextureArea = getContentProjection();

        // BG Color
        ofFill();
        ofSetColor(viewportBgColor);
        ofDrawRectangle(cvp);

        // Checkerboard
        // Fixme: very heavy on the GPU, implement this with a pixelshader ? Or a static image ?
        if(bDrawViewportCheckerboard){
            float diff = (viewportBgColor.getLightness()>0.5) ? -.2 : .2;
            const ofFloatColor checkerColor = {viewportBgColor.r+diff, viewportBgColor.g+diff, viewportBgColor.b+diff, viewportBgColor.a};
            ofSetColor(checkerColor);
            // Every 10 px with zoom
            const float scale = 10.0;
            for(float x = 0; x<cvp.width; x+= scale){
                for(float y = 0; y<cvp.height; y+= scale){
                    if(glm::mod(glm::round(y/scale+x/scale), 2.f)==0){
                        // Don't over-flow !
                        const glm::vec2 size = { glm::min(cvp.width-x, scale), glm::min(cvp.height-y, scale)};
                        ofDrawRectangle(cvp.x+x, cvp.y+y, size.x, size.y);
                    }
                }
            }
        }

        // Draw the canvas
        ofFill();
        ofSetColor(ofColor::white);
        fbo.getTexture(0).setTextureWrap(texRepeatMode, texRepeatMode);
        fbo.getTexture(0).setTextureMinMagFilter(texSampleMode, texSampleMode);
        fbo.getTexture(0).drawSubsection(cvp.x, cvp.y, cvp.width, cvp.height, ctextureArea.x, ctextureArea.y, ctextureArea.width, ctextureArea.height);
    }
    if(bDrawViewportOutline){
        ofNoFill();
        ofSetLineWidth(2);
        ofSetColor(ofColor::red);
        ofDrawRectangle(viewportRect);
    }
    if(bDrawContentOutline){
        float viewZoom = getViewZoom();
        float scale = getScale();
        ofRectangle contentArea = {
            getViewportRect().getTopLeft()+(getViewTranslation()*-1.f*viewZoom/scale),
            getCanvasSize().x/scale*viewZoom,
            getCanvasSize().y/scale*viewZoom
        };
        ofNoFill();
        ofSetLineWidth(2);
        ofSetColor(ofColor::red);
        ofDrawRectangle(contentArea);
    }
    ofPopStyle();
}

// Submits ImGui commands
void ofxSimpleAppCanvas::drawGuiSettings(){
    ImGui::Dummy({ofxSA_UI_MARGIN,ofxSA_UI_MARGIN});
    ImGui::SeparatorText("Output Canvas");

    bool reAllocate = false;
    if(fbo.isAllocated()) ImGui::Text("Resolution: %u x %u (ratio=%.2f)", getCanvasResolutionX(), getCanvasResolutionY(), (((float)getCanvasResolutionX())/getCanvasResolutionY()));
    else ImGui::Text("Resolution: [Not Allocated!]");
    //InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
    static unsigned int pixelSteps[2] = {1, 10}; // Slow + Fast steps
    if(ImGui::DragFloat("Rendering scale", &scale, 0.05f, -10, 10, "%.2f", ImGuiSliderFlags_None)){
        reAllocate=true;
    }
    ImGui::InputScalar("Width", ImGuiDataType_U32, &width, (void*)&pixelSteps[0], (void*)&pixelSteps[1], "%u px");
    if(ImGui::IsItemDeactivatedAfterEdit()){
        reAllocate=true;
    }
    ImGui::InputScalar("Height", ImGuiDataType_U32, &height, (void*)&pixelSteps[0], (void*)&pixelSteps[1], "%u px");
    if(ImGui::IsItemDeactivatedAfterEdit()){
        reAllocate=true;
    }
    if(reAllocate){
        setCanvasSize(width, height, scale);
    }
    if(ImGui::TreeNode("Canvas Viewport")){
        ImGui::BulletText("Position=[%.0f, %.0f]", viewportRect.x, viewportRect.y);
        ImGui::BulletText("Size=%.0f x %.0f", viewportRect.width, viewportRect.height);
        ImGui::BulletText("Ratio=%.2f", viewportRect.width/viewportRect.height);
        ImGui::Checkbox("Highlight viewport", &bDrawViewportOutline);
        ImGui::Checkbox("Outline content borders", &bDrawContentOutline);
        ImGui::ColorEdit4("Viewport background", &viewportBgColor[0]);
        ImGui::Checkbox("Checkerboard as viewport bg", &bDrawViewportCheckerboard);

        ImGui::Dummy({ofxSA_UI_MARGIN,ofxSA_UI_MARGIN});
        ImGui::SeparatorText("Rendering");
        ImGuiEx::glSampleModeImGuiCombo("Sampling", texSampleMode);
        ImGuiEx::glTexRepeatModeImGuiCombo("Repeat", texRepeatMode);
        ImGui::Checkbox("Flip FBO texture", &fbo.getTexture().getTextureData().bFlipTexture);
        ImGui::TreePop();
    }
#if ofxSA_CANVAS_OUTPUT_EXTRA_STANDALONE_WINDOW == 1
    if(ImGui::TreeNode("Canvas Secondary Output Window")){
        ImGui::TextDisabled("Experimental feature !");
        if(ImGui::Checkbox("Output canvas to an extra window", &bRenderExtraCanvasWindow)){
        }

        ImGui::Text("Window set   : %s", extraCanvasOutputWindowPtr?"yes":"no");
        ImGui::Text("Window ready : %s", (extraCanvasOutputWindowPtr && extraCanvasOutputWindowPtr->getGLFWWindow())?"yes":"no");
        ImGui::Text("Winodw size  : %i x %i", (extraCanvasOutputWindowPtr)?extraCanvasOutputWindowPtr->getWidth():0, (extraCanvasOutputWindowPtr)?extraCanvasOutputWindowPtr->getHeight():0);
        if(extraCanvasOutputWindowPtr){
            bool bIsFullscreen = extraCanvasOutputWindowPtr->getSettings().windowMode == OF_FULLSCREEN;
            if(ImGui::Checkbox("Fullscreen", &bIsFullscreen)){
                extraCanvasOutputWindowPtr->setFullscreen(bIsFullscreen);
            }
            static bool bIsVSync = true;// = extraCanvasOutputWindowPtr-> == OF_FULLSCREEN;
            if(ImGui::Checkbox("V-Sync", &bIsVSync)){
                extraCanvasOutputWindowPtr->setVerticalSync(bIsVSync);
            }
            int monitorNum = extraCanvasOutputWindowPtr->getSettings().monitor;
            auto glfwWin = extraCanvasOutputWindowPtr->getGLFWWindow();
            GLFWmonitor* glfwActiveMonitor = nullptr;
            const char* glfwActiveMonitorName = nullptr;
            if(glfwWin){
                glfwActiveMonitor = glfwGetWindowMonitor(glfwWin);
                if(glfwActiveMonitor){
                    const GLFWvidmode* mode = glfwGetVideoMode(glfwActiveMonitor);
                    int width_mm, height_mm;
                    glfwGetMonitorPhysicalSize(glfwActiveMonitor, &width_mm, &height_mm);
                    const char* name = glfwGetMonitorName(glfwActiveMonitor);

                    ImGui::Text("Window monitor : %s (%i x %i px)", name?name:"[no name]", width_mm, height_mm );// todo !
                }
            }
            int count = 0;
            const auto monitors = glfwGetMonitors(&count);
            if(monitors){
                if(ImGui::BeginCombo("Assign to monitor:", glfwActiveMonitorName)){
                    for(int i = 0; i<count; i++){
                        GLFWmonitor* monitor = monitors[i];
                        const bool isActive = (glfwActiveMonitor == monitor);
                        ImGui::PushID(monitor);
                        int w,h;//,x,y;
                        glfwGetMonitorPhysicalSize(monitor,&w,&h);
                        //glfwGetMonitorPos(monitor,&x,&y);
                        ImGui::Text("%2i - ", i); ImGui::SameLine();
                        const char* monitorName = glfwGetMonitorName(monitor);
                        if(ImGui::Selectable(monitorName?monitorName:"[no name]")){
                            glfwSetWindowMonitor(glfwWin, monitor, 0, 0, width, height, 0);
                        }
                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (isActive)
                            ImGui::SetItemDefaultFocus();

                        ImGui::SameLine();
                        ImGui::TextDisabled("%i x %i", w, h);
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
            }

            if(ImGui::Button("Close window")){
                extraCanvasOutputWindowPtr->setWindowShouldClose();
                extraCanvasOutputWindowPtr = nullptr; // triggers shared_ptr destroy ?
            }
        }
        else {
            if(ImGui::Button("Create window")){
                if(auto cur = ofGetCurrentWindow()){
                    if(auto curGlfw = std::static_pointer_cast<ofAppGLFWWindow>(cur)){
                        setupSecondaryMonitor(curGlfw);
                    }
                }
            }
        }
        ImGui::TreePop();
    }
#endif
}

// Viewport controls, ImGui HUD
void ofxSimpleAppCanvas::drawGuiViewportHUD(){

    // Start a new window space
    static ImGuiWindowFlags windowFlags =
            //ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoFocusOnAppearing|
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {ofxSA_UI_MARGIN*.5f,4});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2,0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    //glm::vec2 hudSize = { viewportRect.width, ImGui::GetFrameHeight());
    const float hudHeight = ImGui::GetFrameHeightWithSpacing()+4; // fixme hardcoded stuff
    ImVec4 hudRect = { viewportRect.x, viewportRect.y+viewportRect.height-hudHeight, viewportRect.width, hudHeight };
    ImVec2 hudPosScreen = { hudRect.x, hudRect.y };

    // Make hudrect absolute screen coords ?
    if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable ){
        hudRect.x += ofGetWindowPositionX();
        hudRect.y += ofGetWindowPositionY();
    }


    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {hudRect.z, hudRect.w});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetColorU32(ImGuiCol_WindowBg, .5));
    ImGui::SetNextWindowPos({hudRect.x, hudRect.y}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({hudRect.z, hudRect.w}, ImGuiCond_Always);
    //ImGui::SetNextWindowSizeConstraints({hudRect.z, hudRect.w}, {hudRect.z, hudRect.w});
    if(ImGui::Begin("Canvas Viewport HUD", NULL, windowFlags)){
        ImGui::PushItemWidth(50);
        //ImGui::SetWindowSize({hudRect.z, hudRect.w}, ImGuiCond_Always);
        //ImDrawList* dl = ImGui::GetWindowDrawList();

        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Spacing();

        ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::Text("Viewport ");
        ImGui::EndDisabled();

        ImGui::SameLine();
        const char* items[] = { "Manual", "Cover", "Contain" };
        if (ImGui::BeginCombo("##canvasDrawMode", (const char*)items[contentDrawMode])){
            for (int n = 0; n < IM_ARRAYSIZE(items); n++){
                const bool is_selected = (contentDrawMode == n);
                if (ImGui::Selectable(items[n], is_selected)){
                    if(n==1) contentDrawMode = CanvasDrawMode_AutoCover;
                    else if(n==2) contentDrawMode = CanvasDrawMode_AutoContain;
                    else contentDrawMode = CanvasDrawMode_Manual;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::Spacing();

        if(contentDrawMode == CanvasDrawMode_Manual){
            ImGui::SameLine();
            if(ImGui::Button("-##zoom")){
                viewZoom = (round(viewZoom/0.25)*0.25 - 0.25);
            }
            ImGui::SameLine();
            if(ImGui::Button("+##zoom")){
                viewZoom = (round(viewZoom/0.25)*0.25 + 0.25);
            }
            ImGui::SameLine();
            ImGui::DragFloat("Zoom##canvas", &viewZoom, 0.001f, 0, 10);
            if(ImGui::IsItemClicked(ImGuiMouseButton_Right)){
                ImGui::OpenPopup("canvas-zoom-popup");
            }
            if(ImGui::BeginPopup("canvas-zoom-popup")){
                ImGui::SeparatorText("Zoom");
                if(ImGui::MenuItem("Set to contain")){

                }
                if(ImGui::MenuItem("Set to cover")){

                }
                if(ImGui::MenuItem("Set to 100%")){

                }

                ImGui::Dummy({ofxSA_UI_MARGIN,ofxSA_UI_MARGIN});
                ImGui::SeparatorText("Rendering");
                ImGuiEx::glSampleModeImGuiCombo("Sampling", texSampleMode);
                ImGuiEx::glTexRepeatModeImGuiCombo("Repeat", texRepeatMode);
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            ImGui::Spacing();
        }

        ImGui::SameLine();
        ImGui::Button("[+]##canvasview");
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            viewTranslation.x += delta.x*.1f;
            viewTranslation.y += delta.y*.1f;
        }
        else if(ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary)){
            if(ImGui::BeginTooltip()){
                ImGui::Text("(drag to translate)");
                ImGui::EndTooltip();
            }
        }
        ImGui::SameLine();
        ImGui::DragFloat("X##canvasviewT", &viewTranslation.x, 1.f, -5000, +5000,"%.0f");
        ImGui::SameLine();
        ImGui::DragFloat("Y##canvasviewT", &viewTranslation.y, 1.f, -5000, +5000,"%.0f");

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        if(ImGui::Button("Reset##canvasview")){
            viewZoom = 1.f;
            viewTranslation.x = 0;
            viewTranslation.y = 0;
        }

        ImGui::PopItemWidth();
    }
    // End window
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);
}

#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
// Load + Save
bool ofxSimpleAppCanvas::populateXmlNode(pugi::xml_node &_node){
    bool ret = true;

    // Resolution
    //_node.append_child("width").text().set(width);
    //_node.append_child("height").text().set(height);
    //_node.append_child("scale").text().set(scale);

    // Resolution
    pugi::xml_node resolutionNode = ofxPugiXml::getOrAppendNode(_node, "resolution");
    ret *= (bool) resolutionNode;
    if(resolutionNode){
        ret *= ofxPugiXml::setNodeAttribute(resolutionNode, "width", width);
        ret *= ofxPugiXml::setNodeAttribute(resolutionNode, "height", height);
        ret *= ofxPugiXml::setNodeAttribute(resolutionNode, "scale", scale);
    }

    // View
    pugi::xml_node viewNode = ofxPugiXml::getOrAppendNode(_node, "view");
    ret *= (bool) viewNode;
    if(viewNode){
        pugi::xml_node zoomNode = ofxPugiXml::getOrAppendNode(viewNode, "zoom");
        ret *= (bool) zoomNode;
        if(zoomNode){
            ret *= ofxPugiXml::setNodeAttribute<int>(zoomNode, "mode", (int&)contentDrawMode);
            ret *= ofxPugiXml::setNodeAttribute(zoomNode, "scale", viewZoom);
            ret *= ofxPugiXml::setNodeAttribute(zoomNode, "translation", viewTranslation);
        }

        pugi::xml_node bgNode = ofxPugiXml::getOrAppendNode(viewNode, "background");
        ret *= (bool) bgNode;
        if(bgNode){
            ret *= ofxPugiXml::setNodeAttribute(bgNode, "checkerboard", bDrawViewportCheckerboard);
            ret *= ofxPugiXml::setNodeAttribute(bgNode, "color", viewportBgColor);
        }

        pugi::xml_node renderingNode = ofxPugiXml::getOrAppendNode(viewNode, "rendering");
        ret *= (bool) renderingNode;
        if(renderingNode){
            ret *= ofxPugiXml::setNodeAttribute<int>(renderingNode, "repeat", texRepeatMode);
            ret *= ofxPugiXml::setNodeAttribute<int>(renderingNode, "sampling", texSampleMode);
            ret *= ofxPugiXml::setNodeAttribute<int>(renderingNode, "flip-texture", fbo.getTexture().getTextureData().bFlipTexture);
        }

        pugi::xml_node drawNode = ofxPugiXml::getOrAppendNode(viewNode, "draw");
        ret *= (bool) drawNode;
        if(drawNode){
            ret *= ofxPugiXml::setNodeAttribute(drawNode, "content-outline", bDrawContentOutline);
            ret *= ofxPugiXml::setNodeAttribute(drawNode, "viewport-outline", bDrawViewportOutline);
        }

    }

    // Output window
#if ofxSA_CANVAS_OUTPUT_EXTRA_STANDALONE_WINDOW == 1
    pugi::xml_node windowNode = ofxPugiXml::getOrAppendNode(_node, "output-window");
    ret *= (bool) windowNode;

    if(windowNode){
        bool bWindowEnabled = extraCanvasOutputWindowPtr != nullptr && extraCanvasOutputWindowPtr->getGLFWWindow() != nullptr;
        ofxPugiXml::setNodeAttribute(windowNode, "enabled", bWindowEnabled);
        ret *= ofxPugiXml::setNodeAttribute(windowNode, "enable-draw", bRenderExtraCanvasWindow);

        if(bWindowEnabled){
            ret *= ofxPugiXml::setNodeAttribute(windowNode, "width", extraCanvasOutputWindowPtr->getWidth());
            ret *= ofxPugiXml::setNodeAttribute(windowNode, "height", extraCanvasOutputWindowPtr->getHeight());
            ret *= ofxPugiXml::setNodeAttribute(windowNode, "fullscreen", extraCanvasOutputWindowPtr->getWindowMode() == ofWindowMode::OF_FULLSCREEN);
        }
    }
#endif

    return true;
}

bool ofxSimpleAppCanvas::retrieveXmlNode(pugi::xml_node &_node){
    //pugi::xml_node wNode = _node.child("width");
    //pugi::xml_node hNode = _node.child("height");
    //pugi::xml_node sNode = _node.child("scale");
    //pugi::xml_node vmNode = _node.child("viewmode");

    //if(wNode && hNode && sNode){
    //    setCanvasSize(
    //        wNode.text().as_uint(),
    //        hNode.text().as_uint(),
    //        sNode.text().as_float()
    //    );
    //    contentDrawMode = vmNode ? ((CanvasDrawMode) vmNode.text().as_int()) : CanvasDrawMode_Manual;
    //    //std::cout << "Retrieved canvas size ! w="<< width << std::endl;
    //    return true;
    //}
    //return false;

    bool ret = true;

    // Resolution
    pugi::xml_node resolutionNode = _node.child("resolution");
    ret *= (bool) resolutionNode;
    if(resolutionNode){
        ret *= ofxPugiXml::getNodeAttributeValue(resolutionNode, "width", width, (unsigned int) ofGetWidth());
        ret *= ofxPugiXml::getNodeAttributeValue(resolutionNode, "height", height, (unsigned int) ofGetHeight());
        ret *= ofxPugiXml::getNodeAttributeValue(resolutionNode, "scale", scale, 1.f);
        setCanvasSize(width, height, scale);
    }

    // View
    pugi::xml_node viewNode = _node.child("view");
    ret *= (bool) viewNode;
    if(viewNode){
        pugi::xml_node zoomNode = viewNode.child("zoom");
        ret *= (bool) zoomNode;
        if(viewNode){
            ret *= ofxPugiXml::getNodeAttributeValue<int>(zoomNode, "mode", (int&)contentDrawMode, (int)CanvasDrawMode_Manual);
            ret *= ofxPugiXml::getNodeAttributeValue(zoomNode, "scale", viewZoom, 1.f);
            ret *= ofxPugiXml::getNodeAttributeValue(zoomNode, "translation", viewTranslation, glm::vec2(0,0));
        }

        pugi::xml_node bgNode = viewNode.child("background");
        ret *= (bool) bgNode;
        if(viewNode){
            ret *= ofxPugiXml::getNodeAttributeValue(bgNode, "checkerboard", bDrawViewportCheckerboard, false);
            ret *= ofxPugiXml::getNodeAttributeValue(bgNode, "color", viewportBgColor, ofFloatColor::black);
        }

        pugi::xml_node renderingNode = viewNode.child("rendering");
        ret *= (bool) renderingNode;
        if(renderingNode){
            ret *= ofxPugiXml::getNodeAttributeValue<int>(renderingNode, "repeat", (int&)texRepeatMode, (int)GlTexRepeatMode_BorderColor);
            ret *= ofxPugiXml::getNodeAttributeValue<int>(renderingNode, "sampling", (int&)texSampleMode, (int)GlSampleMode_Linear);
            ret *= ofxPugiXml::getNodeAttributeValue(renderingNode, "flip-texture", fbo.getTexture().getTextureData().bFlipTexture, false);
        }

        pugi::xml_node drawNode = viewNode.child("draw");
        ret *= (bool) drawNode;
        if(drawNode){
            ret *= ofxPugiXml::getNodeAttributeValue(drawNode, "content-outline", bDrawContentOutline, false);
            ret *= ofxPugiXml::getNodeAttributeValue(drawNode, "viewport-outline", bDrawViewportOutline, false);
        }
    }

    // Output window
#if ofxSA_CANVAS_OUTPUT_EXTRA_STANDALONE_WINDOW == 1
    pugi::xml_node windowNode = _node.child("output-window");
    ret *= (bool) windowNode;
    if(windowNode){
        bool bWindowEnabled = false;
        ofxPugiXml::getNodeAttributeValue(windowNode, "enabled", bWindowEnabled, false);
        ret *= ofxPugiXml::getNodeAttributeValue(windowNode, "enable-draw", bRenderExtraCanvasWindow, true);

        if(bWindowEnabled){
            // Ensure window is there
            if(!extraCanvasOutputWindowPtr || !extraCanvasOutputWindowPtr->getGLFWWindow()){
                if(auto cur = ofGetCurrentWindow()){
                    if(auto curGlfw = std::static_pointer_cast<ofAppGLFWWindow>(cur)){
                        setupSecondaryMonitor(curGlfw);
                    }
                }
            }
            int width = 0;
            int height = 0;
            bool fs = false;

            ret *= ofxPugiXml::getNodeAttributeValue(windowNode, "width", width);
            ret *= ofxPugiXml::getNodeAttributeValue(windowNode, "height", height);
            ret *= ofxPugiXml::getNodeAttributeValue(windowNode, "fullscreen", fs);

            if(extraCanvasOutputWindowPtr){
                if(fs){
                    if(extraCanvasOutputWindowPtr->getWindowMode() != ofWindowMode::OF_FULLSCREEN){
                        extraCanvasOutputWindowPtr->setFullscreen(true);
                    }
                }
                else if(width!=0 && height !=0){
                    if(extraCanvasOutputWindowPtr->getWindowMode() != ofWindowMode::OF_WINDOW){
                        extraCanvasOutputWindowPtr->setWindowShape(width, height);
                    }
                }
                // Wrong data combination
                else {
                    ofLogNotice("ofxSimpleAppCanvas::retrieveXmlNode") << "Could not restore 2ndary window : Illegal window configuration.";
                    ret *= false;
                }
            }
            else {
                ofLogNotice("ofxSimpleAppCanvas::retrieveXmlNode") << "Could not restore 2ndary window : Wailed creating it !";
            }
        }
        else {
            // Ensure to disable it ?
            if(extraCanvasOutputWindowPtr){
                closeSecondaryWindow();
            }
        }
    }
#endif

    return true;
}
#endif


#include "ofxSimpleAppCanvas.h"

ContentResizeArgs::ContentResizeArgs(unsigned int _width, unsigned int _height, float _scale) :
    width(_width),
    height(_height),
    scale(_scale)
{

}



ofxSimpleAppCanvas::ofxSimpleAppCanvas(unsigned int _width, unsigned int _height){
    setScreenRect();
    setCanvasSize(_width, _height);
}

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
    ofRectangle vp = screenRect;//getViewport();
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

void ofxSimpleAppCanvas::setScreenRect(unsigned int _width, unsigned int _height, unsigned int _x, unsigned int _y){
    screenRect.x=_x;
    screenRect.y=_y;
    screenRect.width=_width;
    screenRect.height=_height;
    bFlagRepaint = true;

    //ofRectangle eventArgs = ofRectangle(width, height);
    ofNotifyEvent(onViewportResize, screenRect, this);
}

ofRectangle ofxSimpleAppCanvas::getScreenRect() const {
    return screenRect;
}

ofRectangle ofxSimpleAppCanvas::getContentProjection() const {
    ofRectangle vp = getScreenRect();
    glm::vec2 contentSize = getCanvasResolution();//blend2d.getSize();
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
        ofFill();
        ofSetColor(ofColor::white);
        fbo.draw(0,0);//screenRect.x, screenRect.y, screenRect.width, screenRect.height);
    }
    if(bDrawScreenRect){
        ofNoFill();
        ofSetLineWidth(2);
        ofSetColor(ofColor::red);
        ofDrawRectangle(screenRect);
    }
    ofPopStyle();
}

// Submits ImGui commands
void ofxSimpleAppCanvas::drawGuiSettings(){
    ImGui::Dummy({ofxSA_UI_MARGIN,ofxSA_UI_MARGIN});
    ImGui::SeparatorText("Output Canvas");

    bool reAllocate = false;
    if(fbo.isAllocated()) ImGui::Text("Resolution: %u x %u", getCanvasResolutionX(), getCanvasResolutionY());
    else ImGui::Text("Resolution: [Not Allocated!]");
    //InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
    static unsigned int pixelSteps[2] = {1, 10}; // Slow + Fast steps
    if(ImGui::DragFloat("Rendering scale", &scale, 0.05f, -10, 10, "%.2f", ImGuiSliderFlags_None)){
        reAllocate=true;
    }
    if(ImGui::InputScalar("Width", ImGuiDataType_U32, &width, (void*)&pixelSteps[0], (void*)&pixelSteps[1], "%u px", ImGuiInputTextFlags_EnterReturnsTrue)){
        reAllocate=true;
    }
    if(ImGui::InputScalar("Height", ImGuiDataType_U32, &height, (void*)&pixelSteps[0], (void*)&pixelSteps[1], "%u px", ImGuiInputTextFlags_EnterReturnsTrue)){
        reAllocate=true;
    }
    if(reAllocate){
        setCanvasSize(width, height, scale);
    }
    ofRectangle screenrect = getScreenRect();
    ImGui::Text("Display Zone: pos=[%.0f, %.0f], size=%.0f x %.0f", screenrect.x, screenrect.y, screenrect.width, screenrect.height);
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

    //glm::vec2 hudSize = { screenRect.width, ImGui::GetFrameHeight());
    float hudHeight = ImGui::GetFrameHeightWithSpacing()+4;
    ImVec4 hudRect = { screenRect.x, screenRect.y+screenRect.height-hudHeight, screenRect.width, hudHeight };

    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {hudRect.z, hudRect.w});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetColorU32(ImGuiCol_WindowBg, .5));
    ImGui::SetNextWindowPos({hudRect.x, hudRect.y}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({hudRect.z, hudRect.w}, ImGuiCond_Always);
    //ImGui::SetNextWindowSizeConstraints({hudRect.z, hudRect.w}, {hudRect.z, hudRect.w});
    if(ImGui::Begin("Canvas Viewport HUD", NULL, windowFlags)){
        ImGui::PushItemWidth(50);
        ImGui::SetWindowSize({hudRect.z, hudRect.w}, ImGuiCond_Always);
        ImDrawList* dl = ImGui::GetWindowDrawList();

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

        ImGui::SameLine();
        if(ImGui::Button("-##zoom")){
            viewZoom = (round(viewZoom/0.25)*0.25 - 0.25);
        }
        ImGui::SameLine();
        if(ImGui::Button("+##zoom")){
            viewZoom = (round(viewZoom/0.25)*0.25 + 0.25);
        }
        //float _viewZoom = viewZoom;
        ImGui::SameLine();
        if(ImGui::DragFloat("Zoom##canvas", &viewZoom, 0.001f, 0, 10)){

        }

        ImGui::SameLine();
        ImGui::Spacing();

        ImGui::SameLine();
        ImGui::Button("[+]##canvasview");
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            viewTranslation.x += delta.x;
            viewTranslation.y += delta.y;
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
    //pugi::xml_node shapesNode = _node.append_child("shapes");

    // Resolution
    _node.append_child("width").text().set(width);
    _node.append_child("height").text().set(height);
    _node.append_child("scale").text().set(scale);

    return true;
}

bool ofxSimpleAppCanvas::retrieveXmlNode(pugi::xml_node &_node){
    pugi::xml_node wNode = _node.child("width");
    pugi::xml_node hNode = _node.child("height");
    pugi::xml_node sNode = _node.child("scale");

    if(wNode && hNode && sNode){
        setCanvasSize(
            wNode.text().as_uint(),
            hNode.text().as_uint(),
            sNode.text().as_float()
        );

        return true;
    }
    return false;
}
#endif

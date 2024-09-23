#include "ofxSimpleAppCanvas.h"

ContentResizeArgs::ContentResizeArgs(unsigned int _width, unsigned int _height, float _scale) :
    width(_width),
    height(_height),
    scale(_scale)
{

}

ofxSimpleAppCanvas::ofxSimpleAppCanvas(unsigned int _width, unsigned int _height){
    setViewportRect();
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
        ofFill();
        ofSetColor(ofColor::white);
        //fbo.draw(0,0);//viewportRect.x, viewportRect.y, viewportRect.width, viewportRect.height);

        ofRectangle cvp = getViewportRect();
        ofRectangle ctextureArea = getContentProjection();
        ofFill();
        ofSetColor(ofColor::white);
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
    if(ImGui::InputScalar("Width", ImGuiDataType_U32, &width, (void*)&pixelSteps[0], (void*)&pixelSteps[1], "%u px", ImGuiInputTextFlags_EnterReturnsTrue)){
        reAllocate=true;
    }
    if(ImGui::InputScalar("Height", ImGuiDataType_U32, &height, (void*)&pixelSteps[0], (void*)&pixelSteps[1], "%u px", ImGuiInputTextFlags_EnterReturnsTrue)){
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
        ImGui::TreePop();
    }
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

            ImGui::SameLine();
            ImGui::Spacing();
        }

        ImGui::SameLine();
        ImGui::Button("[+]##canvasview");
        if(ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            viewTranslation.x += delta.x;
            viewTranslation.y += delta.y;
        }
        else if(ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary)){
            if(ImGui::BeginTooltip()){
                ImGui::Text("(drag to translate)");
            }
            ImGui::EndTooltip();
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

    // View
    _node.append_child("viewmode").text().set(contentDrawMode);

    return true;
}

bool ofxSimpleAppCanvas::retrieveXmlNode(pugi::xml_node &_node){
    pugi::xml_node wNode = _node.child("width");
    pugi::xml_node hNode = _node.child("height");
    pugi::xml_node sNode = _node.child("scale");
    pugi::xml_node vmNode = _node.child("viewmode");

    if(wNode && hNode && sNode){
        setCanvasSize(
            wNode.text().as_uint(),
            hNode.text().as_uint(),
            sNode.text().as_float()
        );
        contentDrawMode = (CanvasDrawMode) vmNode.text().as_int();
        return true;
    }
    return false;
}
#endif

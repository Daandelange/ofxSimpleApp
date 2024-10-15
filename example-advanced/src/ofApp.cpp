#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofxSimpleApp::setup();

    // Init ball similation
    launchBall();

    // Register timeline listeners
#ifdef ofxSA_TIMELINE_ENABLE
    ofAddListener(ofxSA_TIMELINE_GET(timeline).onStart, this, &ofApp::onTimelineRestart);
    ofAddListener(ofxSA_TIMELINE_GET(timeline).onFrameTick, this, &ofApp::onTimelineFrame);
    ofAddListener(ofxSA_TIMELINE_GET(timeline).onSeek, this, &ofApp::onTimelineSeek);
    ofAddListener(ofxSA_TIMELINE_GET(timeline).onPause, this, &ofApp::onTimelinePause);
#endif

    bEnableClearing = false;
    bEnableFading = true;
    bgFadeColor = ofFloatColor(1.0/255, 2.0/255, .5, 0.0);

    // Start with a non-transparent bg
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    canvas.fbo.begin();
    ofClear(0,0,0,255);
    canvas.fbo.end();
#endif

    // Complete message
    ofLogNotice("ofApp::setup") << "Hello, welcome to "<< ofxSA_APP_NAME <<" :) . Setup complete.";
}

//--------------------------------------------------------------
void ofApp::exit(){
    // Remove timeline listeners
#ifdef ofxSA_TIMELINE_ENABLE
    ofRemoveListener(ofxSA_TIMELINE_GET(timeline).onStart, this, &ofApp::onTimelineRestart);
    ofRemoveListener(ofxSA_TIMELINE_GET(timeline).onFrameTick, this, &ofApp::onTimelineFrame);
    ofRemoveListener(ofxSA_TIMELINE_GET(timeline).onSeek, this, &ofApp::onTimelineSeek);
    ofRemoveListener(ofxSA_TIMELINE_GET(timeline).onPause, this, &ofApp::onTimelinePause);
#endif

    // Default exit behaviour
    ofxSimpleApp::exit();
}

//--------------------------------------------------------------
void ofApp::update(){
    ofxSimpleApp::update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    // This calls the parent draw function
    // You can also replicate the function to your needs by not calling the line below.
    // But in most cases it's easier to override drawScene() instead.
    ofxSimpleApp::draw();
}

//--------------------------------------------------------------
void ofApp::drawScene(){
    // Here, if you use the default draw(), rendering happens automatically to either the window or the canvas fbo.

    ofPushStyle();
    ofFill();
    ofSetColor(255,255,255,255);
    int yPos = 20;
    const int lineSpacing = 22;

    // Get timeline data
#ifdef ofxSA_TIMELINE_ENABLE
    ofxSATimeline& tl = ofxSA_TIMELINE_GET(timeline);
    const unsigned int frameNum = tl.getFrameNum();
    const double elapsedSeconds = tl.getElapsedSeconds();
    const double deltaTime = tl.getCounters().tDelta;
#else
    const unsigned int frameNum = ofGetFrameNum();
    const double elapsedSeconds = ofGetElapsedTimef();
    const double deltaTime = 1.0/ofGetFrameRate();
#endif

    // Canvas data
    ofRectangle bounds = getDocumentSize();

    // Draw frame info
    ofDrawBitmapStringHighlight(ofToString("Seconds= ")+ofToString(elapsedSeconds, 2), 20, yPos); yPos+=lineSpacing;
    ofDrawBitmapStringHighlight(ofToString("Frame  = ")+ofToString(frameNum), 20, yPos); yPos+=lineSpacing;

    // Demonstrate Timeline Ramps usage
#ifdef ofxSA_TIMELINE_ENABLE
    ofFill();
    ofSetColor(0,0,0,255);
    ofDrawRectangle(0, yPos, bounds.width, yPos+lineSpacing*6);
    ofSetColor(255,255,255,255);
    ofDrawRectangle(0, yPos, tl.getRamps().barProgress*bounds.width, 20); yPos+=lineSpacing;
    ofDrawBitmapStringHighlight("barProgress", 20, yPos-5);

    ofDrawRectangle(0, yPos, tl.getRamps().barStep*bounds.width, 20); yPos+=lineSpacing;
    ofDrawBitmapStringHighlight("barStep", 20, yPos-5);

    yPos+=lineSpacing;
    ofDrawRectangle(0, yPos, tl.getRamps().beatProgress*bounds.width, 20); yPos+=lineSpacing;
    ofDrawBitmapStringHighlight("beatProgress", 20, yPos-5);
    ofDrawRectangle(0, yPos, tl.getRamps().beatStep*bounds.width, 20); yPos+=lineSpacing;
    ofDrawBitmapStringHighlight("beatStep", 20, yPos-5);


    yPos+=lineSpacing;
    ofDrawRectangle(0, yPos, tl.getRamps().noteProgress*bounds.width, 20); yPos+=lineSpacing;
    ofDrawBitmapStringHighlight("noteProgress", 20, yPos-5);
    ofDrawRectangle(0, yPos, tl.getRamps().noteStep*bounds.width, 20); yPos+=lineSpacing;
    ofDrawBitmapStringHighlight("noteStep", 20, yPos-5);
#endif

    // Demonstrate simulation with delta time
    ballPos += ballVelocity*ballSpeed*glm::vec2(deltaTime, deltaTime);

    // Check bounds
    if(ballPos.x<=bounds.getLeft()+ballSize || ballPos.x>=bounds.getRight()-ballSize){
        ballVelocity.x *= -1.f;
        ballPos.x += ballVelocity.x*ballSpeed*2.f*deltaTime;
    }
    if(ballPos.y<=bounds.getTop()+ballSize || ballPos.y>=bounds.getBottom()-ballSize){
        ballVelocity.y *= -1.f;
        ballPos.y += ballVelocity.y*ballSpeed*2.f*deltaTime;
    }

    ofFill();
#ifdef ofxSA_TIMELINE_ENABLE
    ofDrawCircle(ballPos, ballSize*.6f+ballSize*0.3f*(glm::sin(tl.getRamps().noteProgress*TWO_PI)));
#else
    ofDrawCircle(ballPos, ballSize);
#endif
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawGui(){

    // Draw ofxSimpleApp GUI (recommended)
    ofxSimpleApp::drawGui();

    // This is where you can submit extra custom GUI calls, either to openframeworks or imgui

    // Our custom app window
    if(ImGui::Begin(ofxSA_APP_NAME)){
        ImGui::SeparatorText("Ball Simulation Settings");
        ImGui::DragFloat("Ball Speed", &ballSpeed, 1.f, 1.f, 1000.f, "%.1f");
        ImGui::DragFloat("Ball Size", &ballSize, 1.f, 1.f, 50.f, "%.0f");
        ImGui::Text("Ball pos=[%.0f, %f]", ballPos.x, ballPos.y);
        ImGui::Text("Ball velocity=[%.2f, %.2f]", ballVelocity.x, ballVelocity.y);
        if(ImGui::Button("reLaunch Ball")){
            launchBall();
        }
        ImGui::TextDisabled("When you save this session \nball settings should restore.");
    }
    ImGui::End();

    // Get the application viewport (specially useful when using docking)
    ofRectangle vpRect = ofxSimpleApp::getGuiViewport(false);
    glm::vec2 vpCenter = vpRect.getCenter();

    // Draw some application viewport HUD (inset viewport + corsair)
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CORNER);
    ofSetColor(255,255,255,50);
    ofNoFill();
    static const int margin = 10;
    ofDrawRectangle(
        vpRect.x+margin,
        vpRect.y+margin,
        vpRect.width-2*margin,
        vpRect.height-2*margin
    );
    ofDrawLine(vpCenter.x-margin, vpCenter.y, vpCenter.x+margin, vpCenter.y);
    ofDrawLine(vpCenter.x, vpCenter.y-margin, vpCenter.x, vpCenter.y+margin);
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::launchBall(){
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
    ofRectangle bounds = {0, 0, (float)canvas.getCanvasWidth(), (float)canvas.getCanvasHeight()};
#else
    ofRectangle bounds = ofGetCurrentViewport();
#endif
    ballPos = bounds.getCenter();
    const float rand = ofRandom(0, TWO_PI);
    ballVelocity = {glm::sin(rand), glm::cos(rand)};
}

//--------------------------------------------------------------
#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
bool ofApp::populateXmlSettings(pugi::xml_node& _node){
    // This is where you save your custom XML settings.

    pugi::xml_node ballNode = _node.append_child("ball");
    ballNode.append_attribute("speed").set_value(ballSpeed);
    ballNode.append_attribute("size" ).set_value(ballSize);

    return true;
}

bool ofApp::retrieveXmlSettings(pugi::xml_node& _node){
    // This is where you load your custom XML settings.

    pugi::xml_node ballNode = _node.child("ball");
    if(ballNode){
        ballSpeed = ballNode.attribute("speed").as_float(ballSpeed);
        ballSize  = ballNode.attribute("size" ).as_float(ballSize);
        return true;
    }
    return false;
}
#endif

//--------------------------------------------------------------
#ifdef ofxSA_TIMELINE_ENABLE
bool ofApp::onTimelineRestart(std::size_t& _loopCount){
    ofLogNotice("ofApp::onTimelineRestart") << "Timeline restarting with loopCount=" << _loopCount;
    return true;
}
bool ofApp::onTimelineFrame(ofxSATimeCounters& _counters){
    //ofLogNotice("ofApp::onTimelineFrame") << "Timeline new frame = " << _counters.frameNum;
    return true;
}
bool ofApp::onTimelineSeek(ofxSATimeCounters& _counters){
    ofLogNotice("ofApp::onTimelineSeek") << "Timeline seeked to " << _counters.elapsedSeconds() << " seconds.";
    return true;
}
bool ofApp::onTimelinePause(bool& _paused){
    ofLogNotice("ofApp::onTimelinePause") << "Timeline paused at " << ofxSA_TIMELINE_GET(timeline).getElapsedSeconds() << " seconds.";
    return true;
}
#endif


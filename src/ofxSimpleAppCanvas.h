#pragma once

//include "ofMain.h"
#include "ofxSimpleAppConfig.h"
#include "ofFbo.h"
#include "ofRectangle.h"
#include "ofAppRunner.h"
#include "ofGraphics.h"
#include "imgui.h"
#include "ImHelpers.h" // For ImVec & glm glue

#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
#include "pugixml.hpp"
#endif

// An FBO with repaint support ?

// Extra window support
#if ofxSA_CANVAS_OUTPUT_EXTRA_STANDALONE_WINDOW == 1
    #include "ofAppGLFWWindow.h"
#endif

#define ofxSA_Canvas_AllowedBorder 20.f

enum CanvasDrawMode : int {
    CanvasDrawMode_Manual = 0,
    CanvasDrawMode_AutoCover = 1,
    CanvasDrawMode_AutoContain = 2,
};

struct ContentResizeArgs : public ofEventArgs {
    unsigned int width = 0;
    unsigned int height = 0;
    float scale = 1.f;
    ContentResizeArgs(unsigned int _width, unsigned int _height, float _scale);
};

enum GlSampleMode_ : GLint {
    GlSampleMode_NearestNeighbour   = GL_NEAREST,   // Closest matching
    GlSampleMode_Linear             = GL_LINEAR,    // Weighted average between 4 pixels
};
extern const std::pair<GlSampleMode_, const char*> glSampleModes[2];
const char* glSampleModeGetName(GlSampleMode_ _mode );

enum GlTexRepeatMode_ : GLint {
    GlTexRepeatMode_Repeat         = GL_REPEAT,          // Tile
    GlTexRepeatMode_RepeatMirrored = GL_MIRRORED_REPEAT, // Mirrored
    GlTexRepeatMode_ClampEdge      = GL_CLAMP_TO_EDGE,   // Edge color
    GlTexRepeatMode_BorderColor    = GL_CLAMP_TO_BORDER, // Black
};
extern const std::pair<GlTexRepeatMode_, const char*> glTexRepeatModes[4];
const char* glTexRepeatModeGetName(GlTexRepeatMode_ _mode );

class ofxSimpleAppCanvas {
    public:
        ofxSimpleAppCanvas(unsigned int _width=ofGetWidth(), unsigned int _height=ofGetHeight());

        void setup(){

        };
        void setCanvasSize(unsigned int _width, unsigned int _height, float _scale=1.f);

        // (Unscaled)
        glm::vec2 getCanvasSize() const;
        unsigned int getCanvasWidth() const;
        unsigned int getCanvasHeight() const;
        glm::vec2 getCanvasResolution() const;
        unsigned int getCanvasResolutionX() const;
        unsigned int getCanvasResolutionY() const;

        // Returns current zoom level (in UI)
        float getViewZoom() const;

        // Renderer scale (aka antialiasing)
        float getScale() const;

        glm::vec2 getViewTranslation() const;

        void setViewportRect(unsigned int _width=ofGetWidth(), unsigned int _height=ofGetHeight(), int _x=0, int _y=0);

        ofRectangle getViewportRect() const;
        ofRectangle getContentProjection() const;

        void draw();
        void drawGuiSettings();
        void drawGuiViewportHUD();

        bool bDrawViewportOutline = false;
        bool bDrawContentOutline = false;
        CanvasDrawMode contentDrawMode = CanvasDrawMode_Manual;
        GlSampleMode_ texSampleMode = GlSampleMode_Linear;
        GlTexRepeatMode_ texRepeatMode = GlTexRepeatMode_BorderColor;


#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
		bool populateXmlNode(pugi::xml_node& _node);
		bool retrieveXmlNode(pugi::xml_node& _node);
#endif

#if ofxSA_CANVAS_OUTPUT_EXTRA_STANDALONE_WINDOW == 1 // todo: move this out of canvas space ?
        void drawSecondaryMonitor(ofEventArgs & args);
        void setupSecondaryMonitor(std::shared_ptr<ofAppGLFWWindow> sharedGlfwWindow);
        void closeSecondaryWindow();
        void onSecondaryWindowClose(ofEventArgs& args);
        bool bRenderExtraCanvasWindow = true;
        std::shared_ptr<ofAppGLFWWindow> extraCanvasOutputWindowPtr;
#endif

    private:
        unsigned int width = 0;
        unsigned int height = 0;
        float scale = 1.f; // Antialiasing control
        bool bFlagDirty = false;
        bool bFlagRepaint = false;
        ofFloatColor viewportBgColor = ofColor(0,0,0,255);
        bool bDrawViewportCheckerboard = false; // default off due to heavy GPU usage

        // Viewzone controls
        float viewZoom = 1.f;
        glm::vec2 viewTranslation = {0,0};

    public:
        ofFbo fbo;
    private:
        ofRectangle viewportRect; // draw area

        // Event emitters for listening to updates
    public:
        ofEvent<ContentResizeArgs> onContentResize;
        ofEvent<ofRectangle> onViewportResize; // Arg = Window coordinates
};



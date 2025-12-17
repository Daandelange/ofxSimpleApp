#pragma once

// To get OF constants
//#include "ofConstants.h"

// This file defines constants that are not subject to be changed.

#define ofxSA_XML_ENGINE_PUGIXML 1
#define ofxSA_XML_ENGINE_TINYXML 2

#define ofxSA_GUI_THEME_DARK 0
#define ofxSA_GUI_THEME_LIGHT 1
#define ofxSA_GUI_THEME_CUSTOM 2
#define ofxSA_GUI_THEME_IMGUI_DEFAULT 3

// - - - - -
// Helpers
#define ofxSA_XSTR(x) #x
#define ofxSA_STRINGIFY(x) ofxSA_XSTR(x)
#define ofxSA_APPEND_H(x) x.h
#define ofxSA_VAR_TO_INCLUDE_HEADER(x) ofxSA_STRINGIFY(ofxSA_APPEND_H(x))
#define ofxSA_APPENDONE_I(x) x##1
#define ofxSA_APPENDONE(x) ofxSA_APPENDONE_I(x)

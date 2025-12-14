#pragma once


// - - - -
// UserConfig Template
// This file is ONLY used to configure Qt-Creator projects, as makefiles and Xcode won't include project files while compiling an addon !
// Copy/Paste the code below in your custom ofxSAUserConfig.h and make it suit your needs.

#include "ofxSimpleAppConstants.h"

// - - - -
// ofxSimpleApp: Application Properties
#define ofxSA_APP_NAME "ofxSimpleApp :: Example-advanced"
#define ofxSA_APP_ID "ofxSimpleAppExampleAdvanced" // No spaces, no special characters, serves as an ID.
#define ofxSA_APP_AUTHOR "Daan de Lange"
#define ofxSA_APP_AUTHOR_WEBSITE "https://github.com/Daandelange/ofxSimpleApp"
#define ofxSA_APP_COPYRIGHT_START_YEAR 2023
#define ofxSA_APP_ABOUT ImGui::TextWrapped("An ofxSimpleApp example.\n\n");
#define ofxSA_APP_VERSION_MAJOR 7
#define ofxSA_APP_VERSION_MINOR 7
#define ofxSA_APP_VERSION_PATCH 7

// - - - -
// UI : Functionality
//#define ofxSA_GUI_DEFAULT_HIDDEN false
//#define SHORTCUT_FUNC "CMD"
//#define MOD_KEY OF_KEY_SUPER
//#define MOD_KEY_IMGUI ImGuiMod_Super
//#define ofxSA_GUI_DEFAULT_HIDDEN false
#define ofxSA_UI_DOCKING_ENABLE true
#define ofxSA_UI_DOCKING_ENABLE_DOCKSPACES true
#define ofxSA_UI_VIEWPORTS_ENABLE true
#define ofxSA_UI_RESTORE_STATE true
//#define ofxSA_UI_IMGUI_CONFIGFLAGS_default_on ImGuiConfigFlags_NavEnableGamepad

// - - - -
// UI : Cosmetics
//#define ofxSA_UI_MARGIN 10
//#define ofxSA_UI_LINE_HEIGHT 22
#define ofxSA_GUI_THEME_DEFAULT ofxSA_GUI_THEME_CUSTOM // Use a predefined theme
#define ofxSA_GUI_THEME_CUSTOM_FILE "UnrealTheme.h" // Custom theme file
#define ofxSA_GUI_THEME_CUSTOM_NAME "Unreal Theme"  // Custom theme name
#define ofxSA_GUI_THEME_CUSTOM_CLASS UnrealTheme    // Custom theme class

// - - - -
// Runtime settings
//#define ofxSA_WINDOW_WIDTH  1920
//#define ofxSA_WINDOW_HEIGHT 1200
//#define ofxSA_FPS_CAP 60
//#define ofxSA_FPS_HISTORY_SIZE 120
//#define ofxSA_AUDIO_SAMPLE_RATE 44100
//#define ofxSA_AUDIO_WINDOW_SIZE 512
//#define ofxSA_GL_VERSION_MAJ 4
//#define ofxSA_GL_VERSION_MIN 1
//#define ofxSA_GLES_VERSION 3

// - - - -
// DEBUG
//#define ofxSA_DEBUG

// - - - -
// Features: Opt-Outs
//#define ofxSA_NO_FBO

// - - - -
// Features: Opt-Ins

// Runtime options
#define ofxSA_BACKGROUND_CLEARING
#define ofxSA_BACKGROUND_FADING

// XML Engine
#define ofxSA_XML_ENGINE ofxSA_XML_ENGINE_PUGIXML
//#define ofxSA_XML_FILENAME "Settings.xml"
//#define ofxSA_XML_FOLDER "save_files/"

#define ofxSA_SYPHON_OUTPUT

#define ofxSA_CANVAS_OUTPUT_ENABLE
//#define ofxSA_CANVAS_OUTPUT_DEFAULT_WIDTH 1920
//#define ofxSA_CANVAS_OUTPUT_DEFAULT_HEIGHT 1080
#define ofxSA_CANVAS_OUTPUT_EXTRA_STANDALONE_WINDOW 1

#define ofxSA_TIMELINE_ENABLE
//#define ofxSA_TIMELINE_AUTOSTART true
//#define ofxSA_TIMELINE_SINGLETON true

#define ofxSA_TEXRECORDER_ENABLE
//#define ofxSA_TEXRECORDER_DEFAULT_CODEC "h264"
//#define ofxSA_TEXRECORDER_DEFAULT_FILENAME (ofToString("Recording-")+ofToString(ofGetYear())+"-"+ofToString(ofGetMonth())+"-"+ofToString(ofGetDay()))
//#define ofxSA_TEXRECORDER_DEFAULT_OUTPUT_FOLDER "recordings"
//#define ofxSA_TEXRECORDER_USE_OFXFASTFBOREADER // Todo: Always on for now
//#define DEFAULT_TEXTURE_RECORDER_CONTAINER_EXT ".mkv"

#define ofxSA_TIME_MEASUREMENTS_ENABLE
#define ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY OF_KEY_PAGE_DOWNN
#define ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY_NAME "PGDWN"
//#define ofxSA_TIME_MEASUREMENTS_AUTO_ENABLED false

#define ofxSA_NDI_SENDER_ENABLE

#define ofxSA_QUADWRAPPER_ENABLE

#pragma once

// To get OF constants
#include "ofConstants.h"
#include "ofxSimpleAppConstants.h"

// - - - - -
// User Config
// Always load user config first, then write defaults.
// Usage : define ofxSA_CONFIG_HEADER_FILE in your build system to load a file named "ofxSAUserConfig.h", or define it with a value to use a custom file name : #define ofxSA_CONFIG_HEADER_FILE=myfilename (.h is added automatically)
// No special characters are allowed in the file name, including dashes and periods.
// #define ofxSA_CONFIG_HEADER_FILE ofxSAUserConfig
#ifdef ofxSA_CONFIG_HEADER_FILE
#   define ofxSA_XSTR(x) #x
#   define ofxSA_STRINGIFY(x) ofxSA_XSTR(x)
#   define ofxSA_APPENDONE_I(x) x##1
#   define ofxSA_APPENDONE(x) ofxSA_APPENDONE_I(x)
    // String empty ? --> Use default file name filename
#   if (ofxSA_APPENDONE_I(ofxSA_CONFIG_HEADER_FILE) == 1)
#       pragma message "ofxSA : Loading & sanitising custom config file !"
#       include "ofxSAUserConfig.h"
#   else
    // Or use the define as a file name to include
#       pragma message "ofxSA : Loading & sanitising custom config file with custom name !"
#       include ofxSA_STRINGIFY(ofxSA_CONFIG_HEADER_FILE.h)
#   endif
#else
#   pragma message "ofxSA : No custom user config, loading defaults !"
#endif

//// Enables debugging
//#define ofxSA_DEBUG

// - - - - -
// App Info

// Credentials
#ifndef ofxSA_APP_NAME
#   define ofxSA_APP_NAME "ofxSimpleApp" // Important : No whitespaces, serves as an ID !
#endif
#ifndef ofxSA_APP_AUTHOR
#   define ofxSA_APP_AUTHOR "Anonymous"
#endif
#ifndef ofxSA_APP_AUTHOR_WEBSITE
#   define ofxSA_APP_AUTHOR_WEBSITE "https://github.com/Daandelange/ofxSimpleApp"
#endif
#ifndef ofxSA_APP_COPYRIGHT_START_YEAR
#   define ofxSA_APP_COPYRIGHT_START_YEAR 0
#endif
#ifndef ofxSA_APP_ABOUT
#   define ofxSA_APP_ABOUT ImGui::TextWrapped("...\n\n");
#endif

// Version Management Defaults
#ifndef ofxSA_APP_VERSION_MAJOR
#   define ofxSA_APP_VERSION_MAJOR 0
#endif
#ifndef ofxSA_APP_VERSION_MINOR
#   define ofxSA_APP_VERSION_MINOR 0
#endif
#ifndef ofxSA_APP_VERSION_PATCH
#   define ofxSA_APP_VERSION_PATCH 1
#endif

// - - - - -
// RUNTIME SETTINGS
#ifndef ofxSA_WINDOW_WIDTH
#   define ofxSA_WINDOW_WIDTH  1024
#endif
#ifndef ofxSA_WINDOW_HEIGHT
#   define ofxSA_WINDOW_HEIGHT 1024
#endif
#ifndef ofxSA_FPS_CAP
#   define ofxSA_FPS_CAP 60
#endif
#ifndef ofxSA_FPS_HISTORY_SIZE
#   define ofxSA_FPS_HISTORY_SIZE 120
#endif
#ifndef ofxSA_AUDIO_SAMPLE_RATE
#   define ofxSA_AUDIO_SAMPLE_RATE 44100
#endif
#ifndef ofxSA_AUDIO_WINDOW_SIZE
#   define ofxSA_AUDIO_WINDOW_SIZE 512
#endif
#ifndef ofxSA_GL_VERSION_MAJ
#   define ofxSA_GL_VERSION_MAJ 4
#endif
#ifndef ofxSA_GL_VERSION_MIN
#   define ofxSA_GL_VERSION_MIN 1
#endif
#ifndef ofxSA_GLES_VERSION
#   if defined(TARGET_RASPBERRY_PI)
#       define ofxSA_GLES_VERSION 2
#   else
#       define ofxSA_GLES_VERSION 3
#   endif
#endif
//#define ofxSA_TEXTURE_LIMIT_PX

//#define ofxSA_ENABLE_PROFILING

// XML
// Todo: allow completely disabling saving ?
// One has to be chosen, use ofxXmlSettings by default (tinyXML)
#if !defined(ofxSA_XML_ENGINE)
#   define ofxSA_XML_ENGINE ofxSA_XML_ENGINE_PUGIXML
#endif

// Helper for defining where Pugi/TinyXml is located
#if defined(DEBUG)
#   if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
#       if OF_VERSION_MAJOR <= 0 && OF_VERSION_MINOR >= 10
#          pragma message "ofxSimpleApp is using libPugi shipped within ofxXML since OF 0.10"
#       else
#          pragma message "ofxSimpleApp is using libPugi shipped within ofxPugiXml, install ofxPugiXml if necessary."
#       endif
#   elif ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_TINYXML
#       pragma error "libTinyXml is not yet supported!"
#       if OF_VERSION_MAJOR >= 0 && OF_VERSION_MINOR >= 10
#          pragma message "ofxSimpleApp is using libTinyXml shipped within ofxXML since OF 0.10"
#       else
#          pragma message "ofxSimpleApp is using libTinyXml shipped within ofxPugiXml, install ofxTinyXml if necessary."
#       endif

#   endif
#endif

// Default save file name
// Todo: rename with Default in name
// Todo: A file to load by default ?
// Todo: Split file extension apart ?
#ifndef ofxSA_XML_FILENAME
#   define ofxSA_XML_FILENAME "Settings.xml"
#endif
// Folder for saving and loading multiple files
#ifndef ofxSA_XML_FOLDER
//  Must have tailing slash !
#   define ofxSA_XML_FOLDER "save_files/"
#endif


// - - - - -
// COSMETICS

// Custom theme
#ifdef ofxSA_GUI_THEME_CUSTOM_FILE
#   pragma message "ofxSA : You choose to use a custom theme, ensure that the file exists !"
#   ifndef ofxSA_GUI_THEME_CUSTOM_NAME // Ensure default theme name
#       define ofxSA_GUI_THEME_CUSTOM_NAME "Custom Theme"
#   endif
#else
#   undef ofxSA_GUI_THEME_CUSTOM_NAME
#endif

// Default theme
#ifndef ofxSA_GUI_THEME_DEFAULT
#   ifdef ofxSA_GUI_THEME_CUSTOM_FILE // Default to custom theme if set
#       define ofxSA_GUI_THEME_DEFAULT ofxSA_GUI_THEME_CUSTOM
#   else
#       define ofxSA_GUI_THEME_DEFAULT ofxSA_GUI_THEME_DARK
#   endif
#endif

// Default gui state
#ifndef ofxSA_GUI_DEFAULT_HIDDEN
#   define ofxSA_GUI_DEFAULT_HIDDEN false
#endif


#ifndef ofxSA_GUI_THEME
//#   define ofxSA_GUI_THEME ofxSA_GUI_THEME_LIGHT// Light theme
#else
#   pragma message "ofxSA : You choose to use a custom theme, ensure that the file exists !"
#endif
#ifndef ofxSA_UI_MARGIN
#   define ofxSA_UI_MARGIN 10
#endif
#ifndef ofxSA_UI_LINE_HEIGHT
#   define ofxSA_UI_LINE_HEIGHT 22
#endif
// todo: Gui scale for DPI ?

// ImGui Options
#ifndef ofxSA_UI_DOCKING_ENABLE
#   define ofxSA_UI_DOCKING_ENABLE 1 // Enabled by default
#endif
#if ofxSA_UI_DOCKING_ENABLE == 1
#   define ofxSA_UI_IMGUI_CONFIGFLAGS_docking ImGuiConfigFlags_DockingEnable
#else
#   define ofxSA_UI_IMGUI_CONFIGFLAGS_docking ImGuiConfigFlags_None
#endif

#ifndef ofxSA_UI_VIEWPORTS_ENABLE
#   define ofxSA_UI_VIEWPORTS_ENABLE 0
#endif
#if ofxSA_UI_VIEWPORTS_ENABLE == 1
#   define ofxSA_UI_IMGUI_CONFIGFLAGS_vp ImGuiConfigFlags_ViewportsEnable
#else
#   define ofxSA_UI_IMGUI_CONFIGFLAGS_vp ImGuiConfigFlags_None
#endif

// Set default restore state (creates savefile in data folder)
#ifndef ofxSA_UI_RESTORE_STATE
#   ifdef ofxSA_UI_DOCKING_ENABLE
#       define ofxSA_UI_RESTORE_STATE 1
#   else
#       define ofxSA_UI_RESTORE_STATE 0
#   endif
#endif

// Saving & Loading functionality


// Build default imgui flags from options
#ifndef ofxSA_UI_IMGUI_CONFIGFLAGS_default_on
#   define ofxSA_UI_IMGUI_CONFIGFLAGS_default_on ImGuiConfigFlags_NavEnableGamepad
#endif
#ifndef ofxSA_UI_IMGUI_CONFIGFLAGS
#   define ofxSA_UI_IMGUI_CONFIGFLAGS ofxSA_UI_IMGUI_CONFIGFLAGS_default_on | ofxSA_UI_IMGUI_CONFIGFLAGS_docking | ofxSA_UI_IMGUI_CONFIGFLAGS_vp
#endif

// Enable docking space
// Force-disable when no docking
#if ofxSA_UI_DOCKING_ENABLE == 0
#   ifdef ofxSA_UI_DOCKING_ENABLE_DOCKSPACES
#       undef ofxSA_UI_DOCKING_ENABLE_DOCKSPACES
#   endif
// Defaults to docking enabled setting
#elif !defined(ofxSA_UI_DOCKING_ENABLE_DOCKSPACES)
#   define ofxSA_UI_DOCKING_ENABLE_DOCKSPACES ofxSA_UI_DOCKING_ENABLE
#endif

// cleanup temps
//#undef oofxSA_UI_IMGUI_CONFIGFLAGS_docking
//#undef ofxSA_UI_IMGUI_CONFIGFLAGS_vp
//#undef ofxSA_UI_IMGUI_CONFIGFLAGS_default_on

// - - - - -
// Optional opt-ins
//#define ofxSA_SYPHON_OUTPUT

// Sanitize Syphon setting (force-disable on unsupported platforms)
#ifdef ofxSA_SYPHON_OUTPUT
    // Only allow on osx platforms
#   ifndef TARGET_OSX
#       pragma message "Warning: ofxSA_SYPHON_OUTPUT is only allowed on macos !"
#       undef ofxSA_SYPHON_OUTPUT
#   endif
#endif

// Sanitize Canvas setting (force-disable on unsupported platforms)
#ifdef ofxSA_CANVAS_OUTPUT_ENABLE
#   ifndef ofxSA_CANVAS_OUTPUT_DEFAULT_WIDTH
#       define ofxSA_CANVAS_OUTPUT_DEFAULT_WIDTH 1920
#   endif
#   ifndef ofxSA_CANVAS_OUTPUT_DEFAULT_HEIGHT
#       define ofxSA_CANVAS_OUTPUT_DEFAULT_HEIGHT 1080
#   endif
#endif

// Timeline settings
// tmp enable by default
#ifdef ofxSA_TIMELINE_ENABLE
//  Enable autostart by default
#   ifndef ofxSA_TIMELINE_AUTOSTART
#       define ofxSA_TIMELINE_AUTOSTART true
#   endif
#   ifndef ofxSA_TIMELINE_SINGLETON
#       define ofxSA_TIMELINE_SINGLETON true
#   endif
//  Helper for getting the timeline instance
#   if ofxSA_TIMELINE_SINGLETON
#       define ofxSA_TIMELINE_GET(varname) ofxSATimeline::getTimeline()
#	else
#       define ofxSA_TIMELINE_GET(varname) varname
#   endif
#else
#   undef ofxSA_TIMELINE_AUTOSTART
#   undef ofxSA_TIMELINE_SINGLETON
#   undef ofxSA_TIMELINE_GET
#endif

// Texture Recorder
// tmp enabled by default
#ifdef ofxSA_TEXRECORDER_ENABLE
//  Set default codec
#   ifndef ofxSA_TEXRECORDER_DEFAULT_CODEC
#       define ofxSA_TEXRECORDER_DEFAULT_CODEC "libx264"
#   endif
#   ifndef ofxSA_TEXRECORDER_DEFAULT_FILENAME
#       define ofxSA_TEXRECORDER_DEFAULT_FILENAME (ofToString("Recording-")+ofToString(ofGetYear())+"-"+ofToString(ofGetMonth())+"-"+ofToString(ofGetDay()))
#   endif
#   ifndef ofxSA_TEXRECORDER_DEFAULT_OUTPUT_FOLDER
#       define ofxSA_TEXRECORDER_DEFAULT_OUTPUT_FOLDER "recordings"
#   endif
// tmp enabled by default
#   ifndef ofxSA_TEXRECORDER_USE_OFXFASTFBOREADER
#       define ofxSA_TEXRECORDER_USE_OFXFASTFBOREADER
#       endif
#endif

// ofxTimeMeasurements
#ifdef ofxSA_TIME_MEASUREMENTS_ENABLE
// todo...
#endif

// Todo :
// - Optional : debugging disable, logging disable,
// - App expiration date for distributing prototypes ?
// - Tracy debugger integration


// Platform variables
// todo: prefix ofxSa_ namespace !
#if !defined(SHORTCUT_FUNC) && !defined(MOD_KEY)
#   if defined(TARGET_OSX)
#       define SHORTCUT_FUNC "CMD"
#       define MOD_KEY OF_KEY_SUPER
#       define MOD_KEY_IMGUI ImGuiMod_Super
//#elif defined(TARGET_LINUX)
//    #define SHORTCUT_FUNC "CTRL"
//    #define MOD_KEY OF_KEY_CONTROL
//#elif defined(TARGET_WIN32)
//    #define SHORTCUT_FUNC "CTRL"
//    #define MOD_KEY OF_KEY_CONTROL
#   else
#       define SHORTCUT_FUNC "CTRL"
#       define MOD_KEY OF_KEY_CONTROL
#       define MOD_KEY_IMGUI ImGuiMod_Ctrl
#   endif
#endif

// - - - -
// UserConfig Template
// Copy/Paste the code below in your custom ofxSAUserConfig.h and make it suit your needs.

//#include "ofxSimpleAppConstants.h"

// - - - -
// ofxSimpleApp: Application Properties
//#define ofxSA_APP_NAME "myAppName"
//#define ofxSA_APP_AUTHOR "Anonymous"
//#define ofxSA_APP_AUTHOR_WEBSITE "https://github.com/Daandelange/ofxSimpleApp"
//#define ofxSA_APP_COPYRIGHT_START_YEAR 2023
//#define ofxSA_APP_ABOUT ImGui::TextWrapped("An ofxSimpleApp example.\n\n");
//#define ofxSA_APP_VERSION_MAJOR 0
//#define ofxSA_APP_VERSION_MINOR 0
//#define ofxSA_APP_VERSION_PATCH 1

// - - - -
// UI : Functionality
//#define ofxSA_GUI_DEFAULT_HIDDEN false
//#define SHORTCUT_FUNC "CMD"
//#define MOD_KEY OF_KEY_SUPER
//#define MOD_KEY_IMGUI ImGuiMod_Super
//#define ofxSA_GUI_DEFAULT_HIDDEN false
//#define ofxSA_UI_DOCKING_ENABLE true
//#define ofxSA_UI_DOCKING_ENABLE_DOCKSPACES true
//#define ofxSA_UI_VIEWPORTS_ENABLE true
//#define ofxSA_UI_RESTORE_STATE true
//#define ofxSA_UI_IMGUI_CONFIGFLAGS_default_on ImGuiConfigFlags_NavEnableGamepad

// - - - -
// UI : Cosmetics
//#define ofxSA_UI_MARGIN 10
//#define ofxSA_UI_LINE_HEIGHT 22
//#define ofxSA_GUI_THEME_DEFAULT ofxSA_GUI_THEME_DARK // Use a predefined theme
//#define ofxSA_GUI_THEME_CUSTOM_FILE "myTheme.h" // Custom theme
//#define ofxSA_GUI_THEME_CUSTOM_NAME "My Theme" // Custom theme

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
// Features: Opt-Outs
//#define ofxSA_NO_FBO

// - - - -
// Features: Opt-Ins

// XML Engine
//#define ofxSA_XML_ENGINE ofxSA_XML_ENGINE_PUGIXML
//#define ofxSA_XML_FILENAME "Settings.xml"
//#define ofxSA_XML_FOLDER "save_files/"

//#define ofxSA_SYPHON_OUTPUT

//#define ofxSA_CANVAS_OUTPUT_ENABLE
//#define ofxSA_CANVAS_OUTPUT_DEFAULT_WIDTH 1920
//#define ofxSA_CANVAS_OUTPUT_DEFAULT_HEIGHT 1080

//#define ofxSA_TIMELINE_ENABLE
//#define ofxSA_TIMELINE_AUTOSTART true
//#define ofxSA_TIMELINE_SINGLETON true


//#define ofxSA_TEXRECORDER_ENABLE
//#define ofxSA_TEXRECORDER_DEFAULT_CODEC "h264"
//#define ofxSA_TEXRECORDER_DEFAULT_FILENAME (ofToString("Recording-")+ofToString(ofGetYear())+"-"+ofToString(ofGetMonth())+"-"+ofToString(ofGetDay()))
//#define ofxSA_TEXRECORDER_DEFAULT_OUTPUT_FOLDER "recordings"
//#define ofxSA_TEXRECORDER_USE_OFXFASTFBOREADER // Todo: Always on for now
//#define DEFAULT_TEXTURE_RECORDER_CONTAINER_EXT ".mkv"

//#define ofxSA_TIME_MEASUREMENTS_ENABLE


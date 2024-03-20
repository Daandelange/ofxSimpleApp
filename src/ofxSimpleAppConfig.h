#pragma once

// - - - - -
// User Config
// Always load user config first
#ifdef ofxSA_CONFIG_HEADER_FILE
//#include "vectorMapperConstants.h"
//#   include ofxSA_CONFIG_HEADER_FILE // <-- only works using clang !
#   include "vectorMapperConstants.h" // tmp!
#   pragma message "ofxSA : Loading & sanitising custom config file !"
#else
#   pragma message "ofxSA : Loading defaults !"
#endif

//// Enables debugging
//#define ofxSA_DEBUG

// - - - - -
// App Info

// Credentials
#ifndef ofxSA_APP_NAME
#   define ofxSA_APP_NAME "ofxSimpleApp"
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

// Version Management
#ifndef ofxSA_VERSION_MAJOR
#   define ofxSA_VERSION_MAJOR 0
#endif
#ifndef ofxSA_VERSION_MINOR
#   define ofxSA_VERSION_MINOR 0
#endif
#ifndef ofxSA_VERSION_PATCH
#   define ofxSA_VERSION_PATCH 1
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
// One has to be chosen, use ofxXmlSettings by default (tinyXML)
#if !defined(ofxSA_XML_ENGINE_TINYXML) && !defined(ofxSA_XML_ENGINE_PUGIXML)
#   define ofxSA_XML_ENGINE_TINYXML
#endif
#ifndef ofxSA_XML_FILENAME
#   define ofxSA_XML_FILENAME "Settings.xml"
#endif

// - - - - -
// COSMETICS

// Build-in themes
#define ofxSA_GUI_THEME_DARK 0
#define ofxSA_GUI_THEME_LIGHT 1
#define ofxSA_GUI_THEME_CUSTOM 2

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

// Build default imgui flags from options
#define ofxSA_UI_IMGUI_CONFIGFLAGS_default_on ImGuiConfigFlags_NavEnableGamepad
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
    #ifndef TARGET_OSX
        #undef ofxSA_SYPHON_OUTPUT
    #endif
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
#endif

// Texture Recorder
// tmp enabled by default
#define ofxSA_TEXRECORDER_ENABLE
#ifdef ofxSA_TEXRECORDER_ENABLE
//  Set default codec
#   ifndef ofxSA_TEXRECORDER_DEFAULT_CODEC
#       define ofxSA_TEXRECORDER_DEFAULT_CODEC "libx264"
#   endif
#endif

// Todo :
// - Optional : debugging disable, logging disable,
// - App expiration date for distributing prototypes ?
// - Tracy debugger integration


// Platform variables
#if !defined(SHORTCUT_FUNC) && !defined(MOD_KEY)
#   if defined(TARGET_OSX)
#       define SHORTCUT_FUNC "CMD"
#       define MOD_KEY OF_KEY_SUPER
//#elif defined(TARGET_LINUX)
//    #define SHORTCUT_FUNC "CTRL"
//    #define MOD_KEY OF_KEY_CONTROL
//#elif defined(TARGET_WIN32)
//    #define SHORTCUT_FUNC "CTRL"
//    #define MOD_KEY OF_KEY_CONTROL
#   else
#       define SHORTCUT_FUNC "CTRL"
#       define MOD_KEY OF_KEY_CONTROL
#   endif
#endif

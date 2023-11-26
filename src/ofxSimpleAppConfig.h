#pragma once

// - - - - -
// User Config
// Always load user config first
#ifdef ofxSA_CONFIG_HEADER_FILE
//#include "vectorMapperConstants.h"
#   include ofxSA_CONFIG_HEADER_FILE
#   pragma message "ofxSA : Loading custom config file !"
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

#pragma once

// Redirect to our custom config
#include "enigmatikConstants.h"

//// Enables debugging
//#define ofxSA_DEBUG 1
//#define ofxSA_APP_NAME "Rotting JPEG Faces"
//#define ofxSA_APP_AUTHOR "Daan de Lange"
//#define ofxSA_APP_AUTHOR_WEBSITE "https://daandelange.com/"

//// Version Management
//#define ofxSA_VERSION_MAJOR 0
//#define ofxSA_VERSION_MINOR 1
//#define ofxSA_VERSION_PATCH 0

//// Other
//#define ofxSA_FPS_HISTORY_SIZE 120
//#define ofxSA_UI_MARGIN 10
//#define ofxSA_UI_LINE_HEIGHT 22

//#define ofxSA_ENABLE_PROFILING 1

// Optional opt-ins
//#define ofxSA_SYPHON_OUTPUT

// Sanitize Syphon setting (force-disable on unsupported platforms)
#ifdef ofxSA_SYPHON_OUTPUT
    // Only allow on osx platforms
    #ifndef TARGET_OSX
        #undef ofxSA_SYPHON_OUTPUT
    #endif
#endif

//// Todo :
//// - Optional : debugging disable, logging disable,
//// - App expiration date for distributing prototypes ?
///  - Tracy debugger integration


//// Platform variables
//#if defined(TARGET_OSX)
//    #define SHORTCUT_FUNC "CMD"
//    #define MOD_KEY OF_KEY_SUPER
////#elif defined(TARGET_LINUX)
////    #define SHORTCUT_FUNC "CTRL"
////    #define MOD_KEY OF_KEY_CONTROL
////#elif defined(TARGET_WIN32)
////    #define SHORTCUT_FUNC "CTRL"
////    #define MOD_KEY OF_KEY_CONTROL
//#else
//    #define SHORTCUT_FUNC "CTRL"
//    #define MOD_KEY OF_KEY_CONTROL
//#endif

################################################################################
# CONFIGURE PROJECT MAKEFILE (optional)
#   This file is where we make project specific configurations.
################################################################################

################################################################################
# OF ROOT
#   The location of your root openFrameworks installation
#       (default) OF_ROOT = ../../.. 
################################################################################
#OF_ROOT = ../../..
OF_ROOT = /Users/daan/Developer/of_v0.12.0/

################################################################################
# ofxSimpleApp Configuration
# This is used by most IDEs except QtCreator
# Add your defines below, here we use none to demonstrate the minimal example
################################################################################
PROJECT_DEFINES =
# # - - - -
# # ofxSimpleApp: Application Properties
# PROJECT_DEFINES += ofxSA_APP_NAME="ofxSimpleApp :: Example-advanced"
# # Line Below: No spaces, no special characters, serves as an ID.
# PROJECT_DEFINES += ofxSA_APP_ID="ofxSimpleAppExampleSimple"
# PROJECT_DEFINES += ofxSA_APP_AUTHOR="Daan de Lange"
# PROJECT_DEFINES += ofxSA_APP_AUTHOR_WEBSITE="https://github.com/Daandelange/ofxSimpleApp"
# PROJECT_DEFINES += ofxSA_APP_COPYRIGHT_START_YEAR=2023
# #PROJECT_DEFINES += ofxSA_APP_ABOUT=ImGui::TextWrapped("An ofxSimpleApp example.\n\n");
# PROJECT_DEFINES += ofxSA_APP_VERSION_MAJOR=7
# PROJECT_DEFINES += ofxSA_APP_VERSION_MINOR=7
# PROJECT_DEFINES += ofxSA_APP_VERSION_PATCH=7

# - - - -
# # UI : Functionality
# PROJECT_DEFINES += ofxSA_GUI_DEFAULT_HIDDEN false
# PROJECT_DEFINES += SHORTCUT_FUNC "CMD"
# PROJECT_DEFINES += MOD_KEY OF_KEY_SUPER
# PROJECT_DEFINES += MOD_KEY_IMGUI ImGuiMod_Super
# PROJECT_DEFINES += ofxSA_GUI_DEFAULT_HIDDEN false
# PROJECT_DEFINES += ofxSA_UI_DOCKING_ENABLE=true
# PROJECT_DEFINES += ofxSA_UI_DOCKING_ENABLE_DOCKSPACES=true
# PROJECT_DEFINES += ofxSA_UI_VIEWPORTS_ENABLE=true
# PROJECT_DEFINES += ofxSA_UI_RESTORE_STATE=true
# #PROJECT_DEFINES += ofxSA_UI_IMGUI_CONFIGFLAGS_default_on ImGuiConfigFlags_NavEnableGamepad

# # - - - -
# # UI : Cosmetics
# #PROJECT_DEFINES += ofxSA_UI_MARGIN 10
# #PROJECT_DEFINES += ofxSA_UI_LINE_HEIGHT 22
# PROJECT_DEFINES += ofxSA_GUI_THEME_DEFAULT=ofxSA_GUI_THEME_CUSTOM
# PROJECT_DEFINES += ofxSA_GUI_THEME_CUSTOM_FILE="UnrealTheme.h"
# PROJECT_DEFINES += ofxSA_GUI_THEME_CUSTOM_NAME="Unreal Theme"
# PROJECT_DEFINES += ofxSA_GUI_THEME_CUSTOM_CLASS=UnrealTheme

# # - - - -
# # Runtime settings
# #PROJECT_DEFINES += ofxSA_WINDOW_WIDTH  1920
# #PROJECT_DEFINES += ofxSA_WINDOW_HEIGHT 1200
# #PROJECT_DEFINES += ofxSA_FPS_CAP 60
# #PROJECT_DEFINES += ofxSA_FPS_HISTORY_SIZE 120
# #PROJECT_DEFINES += ofxSA_AUDIO_SAMPLE_RATE 44100
# #PROJECT_DEFINES += ofxSA_AUDIO_WINDOW_SIZE 512
# #PROJECT_DEFINES += ofxSA_GL_VERSION_MAJ 4
# #PROJECT_DEFINES += ofxSA_GL_VERSION_MIN 1
# #PROJECT_DEFINES += ofxSA_GLES_VERSION 3
# PROJECT_DEFINES += ofxSA_BACKGROUND_CLEARING
# PROJECT_DEFINES += ofxSA_BACKGROUND_FADING

# # - - - -
# # DEBUG
# #PROJECT_DEFINES += ofxSA_DEBUG

# # - - - -
# # Features: Opt-Outs
# #PROJECT_DEFINES += ofxSA_NO_FBO

# # - - - -
# # Features: Opt-Ins

# # - - - -
# # XML Engine
# PROJECT_DEFINES += ofxSA_XML_ENGINE=ofxSA_XML_ENGINE_PUGIXML
# #PROJECT_DEFINES += ofxSA_XML_FILENAME="Settings.xml"
# #PROJECT_DEFINES += ofxSA_XML_FOLDER="save_files/"

# PROJECT_DEFINES += ofxSA_SYPHON_OUTPUT

# PROJECT_DEFINES += ofxSA_CANVAS_OUTPUT_ENABLE
# #PROJECT_DEFINES += ofxSA_CANVAS_OUTPUT_DEFAULT_WIDTH=1920
# #PROJECT_DEFINES += ofxSA_CANVAS_OUTPUT_DEFAULT_HEIGHT=1080

# PROJECT_DEFINES += ofxSA_TIMELINE_ENABLE
# #PROJECT_DEFINES += ofxSA_TIMELINE_AUTOSTART=true
# #PROJECT_DEFINES += ofxSA_TIMELINE_SINGLETON=true

# PROJECT_DEFINES += ofxSA_TEXRECORDER_ENABLE
# #PROJECT_DEFINES += ofxSA_TEXRECORDER_DEFAULT_CODEC="h264"
# #PROJECT_DEFINES += ofxSA_TEXRECORDER_DEFAULT_FILENAME (ofToString("Recording-")+ofToString(ofGetYear())+"-"+ofToString(ofGetMonth())+"-"+ofToString(ofGetDay()))
# #PROJECT_DEFINES += ofxSA_TEXRECORDER_DEFAULT_OUTPUT_FOLDER "recordings"
# #PROJECT_DEFINES += ofxSA_TEXRECORDER_USE_OFXFASTFBOREADER // Todo: Always on for now
# #PROJECT_DEFINES += DEFAULT_TEXTURE_RECORDER_CONTAINER_EXT ".mkv"

# PROJECT_DEFINES += ofxSA_TIME_MEASUREMENTS_ENABLE
# PROJECT_DEFINES += ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY=OF_KEY_PAGE_DOWNN
# PROJECT_DEFINES += ofxSA_TIME_MEASUREMENTS_TOGGLE_KEY_NAME="PGDWN"
# #PROJECT_DEFINES += ofxSA_TIME_MEASUREMENTS_AUTO_ENABLED false

# PROJECT_DEFINES += ofxSA_NDI_SENDER_ENABLE

################################################################################
# PROJECT ROOT
#   The location of the project - a starting place for searching for files
#       (default) PROJECT_ROOT = . (this directory)
#    
################################################################################
# PROJECT_ROOT = .

################################################################################
# PROJECT SPECIFIC CHECKS
#   This is a project defined section to create internal makefile flags to 
#   conditionally enable or disable the addition of various features within 
#   this makefile.  For instance, if you want to make changes based on whether
#   GTK is installed, one might test that here and create a variable to check. 
################################################################################
# None

################################################################################
# PROJECT EXTERNAL SOURCE PATHS
#   These are fully qualified paths that are not within the PROJECT_ROOT folder.
#   Like source folders in the PROJECT_ROOT, these paths are subject to 
#   exlclusion via the PROJECT_EXLCUSIONS list.
#
#     (default) PROJECT_EXTERNAL_SOURCE_PATHS = (blank) 
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_EXTERNAL_SOURCE_PATHS = 

################################################################################
# PROJECT EXCLUSIONS
#   These makefiles assume that all folders in your current project directory 
#   and any listed in the PROJECT_EXTERNAL_SOURCH_PATHS are are valid locations
#   to look for source code. The any folders or files that match any of the 
#   items in the PROJECT_EXCLUSIONS list below will be ignored.
#
#   Each item in the PROJECT_EXCLUSIONS list will be treated as a complete 
#   string unless teh user adds a wildcard (%) operator to match subdirectories.
#   GNU make only allows one wildcard for matching.  The second wildcard (%) is
#   treated literally.
#
#      (default) PROJECT_EXCLUSIONS = (blank)
#
#		Will automatically exclude the following:
#
#			$(PROJECT_ROOT)/bin%
#			$(PROJECT_ROOT)/obj%
#			$(PROJECT_ROOT)/%.xcodeproj
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_EXCLUSIONS =

################################################################################
# PROJECT LINKER FLAGS
#	These flags will be sent to the linker when compiling the executable.
#
#		(default) PROJECT_LDFLAGS = -Wl,-rpath=./libs
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################

# Currently, shared libraries that are needed are copied to the 
# $(PROJECT_ROOT)/bin/libs directory.  The following LDFLAGS tell the linker to
# add a runtime path to search for those shared libraries, since they aren't 
# incorporated directly into the final executable application binary.
# TODO: should this be a default setting?
# PROJECT_LDFLAGS=-Wl,-rpath=./libs

################################################################################
# PROJECT DEFINES
#   Create a space-delimited list of DEFINES. The list will be converted into 
#   CFLAGS with the "-D" flag later in the makefile.
#
#		(default) PROJECT_DEFINES = (blank)
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_DEFINES = 

################################################################################
# PROJECT CFLAGS
#   This is a list of fully qualified CFLAGS required when compiling for this 
#   project.  These CFLAGS will be used IN ADDITION TO the PLATFORM_CFLAGS 
#   defined in your platform specific core configuration files. These flags are
#   presented to the compiler BEFORE the PROJECT_OPTIMIZATION_CFLAGS below. 
#
#		(default) PROJECT_CFLAGS = (blank)
#
#   Note: Before adding PROJECT_CFLAGS, note that the PLATFORM_CFLAGS defined in 
#   your platform specific configuration file will be applied by default and 
#   further flags here may not be needed.
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_CFLAGS = 

################################################################################
# PROJECT OPTIMIZATION CFLAGS
#   These are lists of CFLAGS that are target-specific.  While any flags could 
#   be conditionally added, they are usually limited to optimization flags. 
#   These flags are added BEFORE the PROJECT_CFLAGS.
#
#   PROJECT_OPTIMIZATION_CFLAGS_RELEASE flags are only applied to RELEASE targets.
#
#		(default) PROJECT_OPTIMIZATION_CFLAGS_RELEASE = (blank)
#
#   PROJECT_OPTIMIZATION_CFLAGS_DEBUG flags are only applied to DEBUG targets.
#
#		(default) PROJECT_OPTIMIZATION_CFLAGS_DEBUG = (blank)
#
#   Note: Before adding PROJECT_OPTIMIZATION_CFLAGS, please note that the 
#   PLATFORM_OPTIMIZATION_CFLAGS defined in your platform specific configuration 
#   file will be applied by default and further optimization flags here may not 
#   be needed.
#
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_OPTIMIZATION_CFLAGS_RELEASE = 
# PROJECT_OPTIMIZATION_CFLAGS_DEBUG = 

################################################################################
# PROJECT COMPILERS
#   Custom compilers can be set for CC and CXX
#		(default) PROJECT_CXX = (blank)
#		(default) PROJECT_CC = (blank)
#   Note: Leave a leading space when adding list items with the += operator
################################################################################
# PROJECT_CXX = 
# PROJECT_CC = 

# Uncomment/comment below to switch between C++11 and C++17 ( or newer ). On macOS C++17 needs 10.15 or above.
export MAC_OS_MIN_VERSION = 10.15
export MAC_OS_CPP_VER = -std=c++17

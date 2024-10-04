# ofxSimpleApp

A multi-purpose application layout for quickly setting up OpenFrameworks projects, powered by ofxImGui.


# Features
- Minimal by default, fine-tuneable using a simple opt-in based config file.
- A basic but highly customiseable application GUI layout with a menu, some common keybindings and metrics to get your app going quickly.
- Save/Load system for retrieving settings using either TinyXml or PugiXml (default).
- A canvas class wrapping a rendering FBO displayed in your window.
- Recording and export options (NDI, Syphon, Recordings).
- Debugging features:
	- An in-app console to keep an eye on the internal logs during runtime.
	- ImGui debuggers for GUI debugging
	- Tracy integration for in depth profiling
	- ofxTimeMeasurements integration for quick profiling
- Easy integration of various addons :
	- ofxSyphon for Syphon output.
	- 

# Requirements
You need the following addons:
- ofxImGui
These are optional addons, depending on your config opt-ins.
- Todo...

# Compatibility
Tested with OpenFrameworks 0.12 on macos.

# Installation
Add ofxSimpleApp and ofxImGui to your project, then inherit your `ofApp` from `ofxSimpleApp` instead of `ofBaseApp`.  
Mow you're ready to go for the minimal experience !

# Configuration
Basically you need to configure which features you'd like to use by enabling their respective config flags, and sometimes add the required addons to your project.
 - Option 1 : Put the required flags into a file `ofxSaUserConfig.h` and tell ofxSA where to find it by adding one single project macro define `ofxSA_CONFIG_HEADER_FILE=ofxSAUserConfig'`.
 - Option 2 : Add the required flags to your project compilation defines.
Please refer to the examples for detailed options.


|-----------|----------|-----------|
|Config flag|Extensions|Description|
|-----------|----------|-----------|
| Test      | Test     | Test      |
|-----------|----------|-----------|

# Examples
- example-simple : The minmal beare bones of ofxSimpleApp.
- example-advanced : The full blown version of ofxSimpleApp using all opt-ins.

# Alternative usage
ofxSimpleApp embeds a few useful classes, you can use them by including the appropriate files from your project. Maybe some addons like ofxImGui are required. Configuration applies.
- ofxSimpleAppTimeline : A timeline and metronom with real-time and offline playback capabilities.

# Future ideas
- A global keyboard shortcut system for registering actions ?

# Contributing
ofxSimpleApp tries to simplify the creative process working with OpenFrameworks.  
Many features and addon integrations are probably missing or to be improved.  
Contributions are welcome, feel free to open an issue or submit a PR.

# License
MIT - Made by [Daan de Lange](https://daandelange.com/)
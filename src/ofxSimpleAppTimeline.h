#pragma once

//#include "ofMain.h"
#include "ofxSimpleAppConfig.h"

#include <chrono>
#include <thread>
#include <cmath>
#include <iostream>
#include "imgui.h"
#include "ofxSimpleAppUtils.h"

#ifdef ofxSA_XML_ENGINE_PUGIXML
#include "pugixml.hpp"
#endif

// - - - - - - - - - -

enum ofxSATimelineMode {
    ofxSATimelineMode_RealTime_Relative,
    ofxSATimelineMode_RealTime_Absolute,
    ofxSATimelineMode_Offline,
};

enum ofxSATimelineLoopMode {
    NoLoop,
    LoopOnce,
    LoopInfinite
};

// - - - - - - - - - -

// A struct representing a time signature
struct ofxSATimeSignature {
    ofxSATimeSignature(unsigned int _bars, unsigned int _noteValue, unsigned int _bpm=120);

    double getBeatDurationSecs() const;
    double getNoteDurationSecs() const;
    double getBarDurationSecs() const;

    void set(unsigned int _bars, unsigned int _noteValue, unsigned int _bpm=0);

public:
    unsigned int bpm;
    unsigned int beatsPerBar; // aka. Measure
    unsigned int notesPerBeat; // aka. NoteValue

    struct {
        double bar;
        double note;
        double beat;
    } durations;

    //unsigned int beatsPerMeasure;
    void updateValues();
};

// Struct with all time counters available
struct ofxSATimeCounters {
    unsigned int frameCount; // Number of rendered frames
    unsigned int frameNum; // Theoretical frame number
    unsigned int loopCount;
    unsigned int beatCount;
    unsigned int barCount;
    unsigned int noteCount;
    double playhead; // In seconds
    const double& elapsedSeconds(){ return playhead; } // Alias
};

// - - - - - - - - - -

struct ofxSATimeRamps {
    friend class ofxSATimeline;

    // Animation loop
    //double loopProgress;

    // Bars
    float barProgress; // (linearly 0 to 1 each bar)
    float barPulse; // (a very short and smoothed impulse on each bar start)
    //double barRandom; //

    // Beats
    float beatProgress; // (linearly 0 to 1 on each beat)
    float beatPulse; // (a very short and smoothed impulse on each beat start)
    float beatStep; //  Beat number, from 0 to 1
    //double beatRandom;

    // Notes
    float noteProgress; // (linearly 0 to 1 on each note)
    float notePulse; // (a very short and smoothed impulse on each note start)
    float noteStep; //  Note number, from 0 to 1
    //double noteRandom;


protected:
    ofxSATimeRamps();

    void initializeRamps();
    void updateRamps(double elapsedSeconds, const ofxSATimeSignature& _ts);
};

// - - - - - - - - - -

#define ofxSATL_Ramp_Hist_Size 120
namespace ImGuiEx{
    void PlotRampHistory( float (&historyEntry)[ofxSATL_Ramp_Hist_Size], const float& newValue, const ImVec2& graphPos, const ImVec2& graphSize, bool bUpdateHist=true);
    void RampGraph(const char* name, float (&historyEntry)[ofxSATL_Ramp_Hist_Size], const float& newValue, bool bUpdateHist=true);
}

// - - - - - - - - - -

// A clock class with playhead functionality and some accessory ramps
// That makes up a timeline
class ofxSATimeline {
#ifdef ofxSA_TIMELINE_SINGLETON
    private:
#else
    public:
#endif
        ofxSATimeline(unsigned int _fps=60, double _duration = 60, ofxSATimelineLoopMode _loopMode = ofxSATimelineLoopMode::NoLoop, unsigned int _beatsPerBar = 4, unsigned int _notesPerBeat = 4, int _beatsPerMinute = 120);

#ifdef ofxSA_TIMELINE_SINGLETON
    public:
        //ofxSATimeline(const ofxSATimeline&) = delete; // not assignable
        ofxSATimeline(ofxSATimeline&&) = delete; // not moveable
        static ofxSATimeline& getTimeline(){
            static ofxSATimeline tl;
            return tl;
        }
#endif
    public:
        // Copy assignment. Use with caution! Can conflict with singleton usage creating a copy.
        ofxSATimeline(const ofxSATimeline& _other) :
            fps(_other.fps),
            duration(_other.duration),
            timeSignature(_other.timeSignature),
            loopMode(_other.loopMode),
            playSpeed(1.0),
            playbackMode(ofxSATimelineMode::ofxSATimelineMode_Offline)
        {
            reset();
            // go to same frame as other ?
            if(_other.isPlaying()){
                goToFrame(_other.getFrameNum());
            }
        }

    // Starts the timeline (aka. play())
    void start();
    void startNextFrame();

    // Function to pause the timeline
    void pause();
    bool isPaused();
    bool isRunning();

    // Function to resume the timeline
    void resume();
    void stop();
    void togglePause();

    // Frame-by-frame, relative
    void nextFrame(int _direction = 1);

    void goToFrame(int _frame, bool _relative=false);

    void goToSeconds(double _seconds, bool _relative=false);

    void setLoop(ofxSATimelineLoopMode loopMode);
    void setDuration(double duration);

    void setDurationFromBeats(int bars, int noteValue);
    void setPlaySpeed(double playSpeed);

    double getPlaySpeed() const;

    // Optional, to update realtime modes in-between frames
    void tickUpdate();

    // At the begining of your frame, tick this. Todo: after frame so update() can prapare data for the next frame ?
    void tickFrame();

private:
    void tickPlayHead(const bool isNewFrame = true);

    // Updates playhead for looping
    void checkLoops();

    // Updates internals and ramps according to playhead
    void updateInternals();

public:
    // todo: make private

    double getElapsedSeconds() const;

    unsigned int getFrameNum() const;

    // Returns the number of rendered frames
    unsigned int getElapsedFrames() const;

    // Returns the number of frames if the composition is finite; 0 otherwise.
    unsigned int getTotalFrames() const;

    // Returns the number of bars if the composition is finite; 0 otherwise.
    unsigned int getTotalBars() const;

    // Returns the number of notes if the composition is finite; 0 otherwise.
    unsigned int getTotalNotes() const;

    double getDuration() const;

    // Playhead position
    double getProgress() const;

    // FPS
    unsigned int getFps() const;

    const ofxSATimeSignature& getTimeSignature() const;
    const ofxSATimeCounters& getCounters() const;
    const ofxSATimeRamps& getRamps() const;

//    double getCurrentBeat() const {
//        double beatsElapsed = getElapsedSeconds() * timeSignature.bpm / 60.0;
//        double beatsPerFrame = static_cast<double>(fps) / 60.0;
//        return std::fmod(beatsElapsed, timeSignature.notesPerBeat) + 1.0; // Beats are 1-indexed
//    }

    // Utils / getters
    bool isPlaying() const;
    ofxSATimelineMode getPlayMode() const;
    void setPlayMode(ofxSATimelineMode _mode);

    // ImGui Helpers
    void drawImGuiPlayControls(bool horizontalLayout = true);

    void drawImGuiTimelineWindow(bool* p_open = nullptr);

#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
	bool populateXmlNode(pugi::xml_node& _node);
	bool retrieveXmlNode(pugi::xml_node& _node);
#endif

private:
    // Composition Settings
    unsigned int fps; // Desired framerate (realtime), Exact framerate (offline)
    double duration;
    ofxSATimeSignature timeSignature;
    ofxSATimeRamps timeRamps;

    // Playback
    bool playing;
    bool paused;
    ofxSATimelineLoopMode loopMode;
    double playSpeed;
    ofxSATimelineMode playbackMode;
    bool bStartNextFrame = false;

    // Counters
    ofxSATimeCounters counters;

    // Internals
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point last_frame_time;
    std::chrono::duration<double> paused_time;
    std::chrono::duration<double> pausedTimes;

    //bool loop_complete;


    void reset();
};

//int main() {
//    Timeline timeline(24, 10.0, LoopMode::LoopInfinite, 4, 120);  // Set your desired parameters

//    timeline.setPlaySpeed(0.5);  // Set the play speed (0.5 for half speed, 2.0 for double speed, etc.)

//    timeline.start();

//    // Let the timeline play for a while
//    for (int i = 0; i < 120; ++i) {
//        timeline.tick();
//        std::cout << "Elapsed Time: " << timeline.getElapsedSeconds()
//                  << " seconds, Current Frame: " << timeline.getCurrentFrame()
//                  << ", Current Beat: " << timeline.getCurrentBeat() << std::endl;

//        // Simulate rendering or processing work
//        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
//    }

//    // Pause the timeline
//    timeline.pause();

//    // Resume after a while
//    for (int i = 0; i < 60; ++i) {
//        timeline.tick();
//        std::cout << "Elapsed Time: " << timeline.getElapsedSeconds()
//                  << " seconds, Current Frame: " << timeline.getCurrentFrame()
//                  << ", Current Beat: " << timeline.getCurrentBeat() << std::endl;
//    }

//    // Resume and let it play to completion
//    timeline.start();
//    while (timeline.isPlaying()) {
//        timeline.tick();
//        std::cout << "Elapsed Time: " << timeline.getElapsedSeconds()
//                  << " seconds, Current Frame: " << timeline.getCurrentFrame()
//                  << ", Current Beat: " << timeline.getCurrentBeat() << std::endl;

//        // Simulate rendering or processing work
//        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
//    }

//    return 0;
//}


//int main() {
//    Timeline timeline(24, 10.0, true, 4, 120);  // Set your desired parameters

//    timeline.start();

//    // Let the timeline play for a while
//    for (int i = 0; i < 120; ++i) {
//        timeline.tick();
//        std::cout << "Elapsed Time: " << timeline.getElapsedSeconds()
//                  << " seconds, Current Frame: " << timeline.getCurrentFrame()
//                  << ", Current Beat: " << timeline.getCurrentBeat() << std::endl;

//        // Simulate rendering or processing work
//        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
//    }

//    // Pause the timeline
//    timeline.pause();

//    // Resume after a while
//    for (int i = 0; i < 60; ++i) {
//        timeline.tick();
//        std::cout << "Elapsed Time: " << timeline.getElapsedSeconds()
//                  << " seconds, Current Frame: " << timeline.getCurrentFrame()
//                  << ", Current Beat: " << timeline.getCurrentBeat() << std::endl;
//    }

//    // Resume and let it play to completion
//    timeline.start();
//    while (timeline.isPlaying()) {
//        timeline.tick();
//        std::cout << "Elapsed Time: " << timeline.getElapsedSeconds()
//                  << " seconds, Current Frame: " << timeline.getCurrentFrame()
//                  << ", Current Beat: " << timeline.getCurrentBeat() << std::endl;

//        // Simulate rendering or processing work
//        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
//    }

//    return 0;
//}


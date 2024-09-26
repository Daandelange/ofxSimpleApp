#include "ofxSimpleAppTimeline.h"


// - - - - - - - - - -

ofxSATimeSignature::ofxSATimeSignature(unsigned int _bars, unsigned int _noteValue, unsigned int _bpm){
    set(_bars, _noteValue, _bpm);
}

double ofxSATimeSignature::getBeatDurationSecs() const {
    return durations.beat;
}

double ofxSATimeSignature::getNoteDurationSecs() const {
    return durations.note;
}

double ofxSATimeSignature::getBarDurationSecs() const {
    return durations.bar;
}

void ofxSATimeSignature::set(unsigned int _bars, unsigned int _noteValue, unsigned int _bpm){
    if(_bpm==0) _bpm = bpm; // Don't change by default
    beatsPerBar = _bars;
    notesPerBeat = _noteValue;
    bpm = _bpm;
    updateValues();
}

void ofxSATimeSignature::updateValues(){
    durations.beat = 60.f/bpm;
    durations.bar = getBeatDurationSecs()*beatsPerBar;
    durations.note = getBeatDurationSecs()/notesPerBeat;
}

// - - - - - - - - - -
ofxSATimeRamps::ofxSATimeRamps() {
    initializeRamps();
}

void ofxSATimeRamps::initializeRamps() {
    barProgress = 0.0;
    barPulse = 0.0;
    barStep = 0.0;
    beatProgress = 0.0;
    beatPulse = 0.0;
    beatStep = 0.0;
    noteProgress = 0.0;
    notePulse = 0.0;
    noteStep = 0.0;
}

void ofxSATimeRamps::updateRamps(double elapsedSeconds, const ofxSATimeSignature& _ts, const double& _duration) {

    // Todo: align playhead on FPS frames before (optionally?), to ensure reaching 0 and 1 in-between frames ???
    // (FPS aren't aligned on the BPM grid)

    // Bars
    double secondsPerBar = _ts.getBarDurationSecs();//60.0 / _ts.bpm * _ts.beatsPerBar;
    barElapsed = elapsedSeconds / secondsPerBar;
    barProgress = std::fmod(barElapsed, 1.0);
    barPulse = (barProgress < 0.04 || barProgress >= 1.0) ? 1.0 : 0.0;
    const float totalBars = _duration > secondsPerBar ? (_duration / secondsPerBar) : (_ts.bpm / _ts.beatsPerBar); // Note: Infinite duration falls back on 1 min
    if(totalBars > 1) barStep = std::fmod( glm::floor(elapsedSeconds / secondsPerBar), totalBars)/(totalBars-1);
    else barStep = 0;

    // Beats
    double secondsPerBeat = _ts.getBeatDurationSecs();//60.0 / _ts.bpm;
    //double beatTime = std::fmod(elapsedSeconds / secondsPerBeat, 1.0);
    beatsElapsed = elapsedSeconds / secondsPerBeat;
    beatProgress = std::fmod(beatsElapsed, 1.0);//beatTime;// / secondsPerBar;
    beatPulse = (beatProgress < 0.01 || beatProgress > 0.99) ? 1.0 : 0.0;
    if(_ts.beatsPerBar > 1) beatStep = std::floor(std::fmod(elapsedSeconds / secondsPerBeat, _ts.beatsPerBar))/(_ts.beatsPerBar-1);//beatProgress * _ts.beatsPerBar;
    else beatStep = 0;

    // Notes
    double secondsPerNote = _ts.getNoteDurationSecs();//secondsPerBeat / _ts.notesPerBeat;
    //double noteTime = std::fmod(elapsedSeconds/ secondsPerNote, 1.0);
    noteElapsed = elapsedSeconds/ secondsPerNote;
    noteProgress = std::fmod(noteElapsed, 1.0);//noteTime / secondsPerNote;
    notePulse = (noteProgress < 0.01 || noteProgress > 0.99) ? 1.0 : 0.0;
    if(_ts.notesPerBeat > 1) noteStep = std::floor(std::fmod(elapsedSeconds / secondsPerNote, _ts.notesPerBeat))/(_ts.notesPerBeat-1);//noteProgress * _ts.notesPerBeat;
    else noteStep = 0;
}

// - - - - - - - - - -

namespace ImGuiEx{
    void PlotRampHistory( float (&historyEntry)[ofxSATL_Ramp_Hist_Size], const float& newValue, const ImVec2& graphPos, const ImVec2& graphSize, bool bUpdateHist){
        float graphStep = graphSize.x / ofxSATL_Ramp_Hist_Size;

        // Sync :
        if(bUpdateHist){
            for(unsigned int i=0; i < ofxSATL_Ramp_Hist_Size-1; i++){
                historyEntry[i] = historyEntry[i+1];
            }
            historyEntry[ofxSATL_Ramp_Hist_Size-1] = newValue;
        }

        // Draw
        ImVec2 prevPos;
        ImDrawList* wdl = ImGui::GetWindowDrawList();
        ImColor graphCol = ImGui::GetColorU32(ImGuiCol_Text);
        for(unsigned int gi=0; gi<ofxSATL_Ramp_Hist_Size; gi++){
            ImVec2 newPos = graphPos+ImVec2(gi*graphStep, graphSize.y - (historyEntry[gi]*graphSize.y));
            //wdl->AddCircleFilled(newPos, 3, IM_COL32(255,0,0,255));
            if(gi > 0) wdl->AddLine(prevPos, newPos, graphCol);

            prevPos = newPos;
        }
    }

    void RampGraph(const char* name, float (&historyEntry)[ofxSATL_Ramp_Hist_Size], const float& newValue, bool bUpdateHist){
        ImColor graphColBorder = ImGui::GetColorU32(ImGuiCol_TextDisabled);
        float graphPosX = 200; // Space for text

        ImVec2 graphSize = ImGui::GetContentRegionAvail();
        graphSize.x -= graphPosX;
        graphSize.y = ImGui::GetFrameHeight()*2.f;

        //wdl->AddRect(graphPos, graphPos+graphSize, IM_COL32(255,0,0,255));

        ImVec2 graphPos = ImGui::GetCursorScreenPos();
        graphPos.x += graphPosX;
        ImGui::Text(name, newValue);
        ImGui::SameLine();
        ImGui::Dummy(graphSize);

        static bool bPauseRamps = false;
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left)){
            bPauseRamps = !bPauseRamps;
        }

        // Border
        ImGui::GetWindowDrawList()->AddRect(graphPos, graphPos+graphSize, graphColBorder);

        //  plot...
        PlotRampHistory(historyEntry, newValue, graphPos, graphSize, bUpdateHist && !bPauseRamps);
    }
}

// - - - - - - - - - -

ofxSATimeline::ofxSATimeline(unsigned int _fps, double _duration, ofxSATimelineLoopMode _loopMode, unsigned int _beatsPerBar, unsigned int _notesPerBeat, int _beatsPerMinute)
    : fps(_fps),
      duration(_duration),
      timeSignature(_beatsPerBar, _notesPerBeat, _beatsPerMinute),
      loopMode(_loopMode),
      playSpeed(1.0),
      playbackMode(ofxSATimelineMode::ofxSATimelineMode_Offline)
{
    reset();
}

const ofxSATimeSignature& ofxSATimeline::getTimeSignature() const {
    return timeSignature;
}

const ofxSATimeCounters& ofxSATimeline::getCounters() const {
    return counters;
}
const ofxSATimeRamps& ofxSATimeline::getRamps() const {
    return timeRamps;
}

void ofxSATimeline::start(unsigned int _offset) {
    reset();
    if(_offset>0) goToFrame(_offset, false, false);
    playing = true;
    //std::cout << "Timeline started." << std::endl;

    onStart.notify(counters.loopCount);
}

void ofxSATimeline::startNextFrame(unsigned int _offset){
    bStartNextFrame = true;
    startNextFrameOffet = _offset;
}

// Function to pause the timeline
void ofxSATimeline::pause() {
    if (!paused) {
        if(!playing) start(); // Ensure play is on
        paused = true;
        last_frame_time = std::chrono::high_resolution_clock::now();
        onPause.notify(paused);
    }
}

bool ofxSATimeline::isPaused(){
    return paused;
}

bool ofxSATimeline::isRunning(){
    return playing && !paused;
}

// Function to resume the timeline
void ofxSATimeline::resume() {
    if (playing && paused) {
        paused = false;
        last_frame_time = std::chrono::high_resolution_clock::now();
        onPause.notify(paused);
    }
}

void ofxSATimeline::stop() {
    reset();
    onStop.notify();
}

void ofxSATimeline::togglePause() {
    if(!isPlaying()) start();
    else if(paused) resume();
    else pause();
}

// Frame-by-frame, relative
void ofxSATimeline::nextFrame(int _direction){
    // Only when paused or speed = 0
    if(playing && _direction!=0){
        // Only when paused or speed = 0
        if(playSpeed==0 || paused){
            goToFrame(_direction , true);
    //                //auto current_time = playhead;//std::chrono::duration<double>(playhead - last_frame_time);//playhead;//std::chrono::high_resolution_clock::now();
    //                double frame_time = 1.0 / fps;

    //                // Update playhead
    //                if (playbackMode == ofxSATimelineMode_RealTime_Absolute || playbackMode == ofxSATimelineMode_RealTime_Relative) {
    //                    playhead += frame_time * _direction;
    //                }
    //                else /*if (playbackMode == ofxSATimelineMode_Offline)*/ {
    //                    playhead = frame_time * ( frameNum + _direction);
    //                }
    //                //auto tmp = std::chrono::seconds((long long)playhead);

    //                // Incremment rendered frames
    //                frameCount++;

    //                // Remember (for rt modes)
    ////                last_frame_time = start_time + std::chrono::time_point<std::chrono::steady_clock, std::chrono::seconds>(playhead);
    //                last_frame_time = start_time + std::chrono::seconds((long long)playhead);

    //                checkLoops();

    //                updateInternals();
        }
    }
}

void ofxSATimeline::goToFrame(int _frame, bool _relative, bool _doPause){
    // Enable play, stay paused (allows setting frame)
    if(!playing && _doPause){
        pause();
    }
    // Set frame
    {
        // Sanitize args
        if(_relative){
            // ignore?
            //_frame =
        }
        else {
            // Do not exceed duration
            _frame = _frame % getTotalFrames();
            // Negative frame indicates distance from end
            if(_frame<0) _frame = getTotalFrames() + _frame;
        }
        //auto current_time = playhead;//std::chrono::duration<double>(playhead - last_frame_time);//playhead;//std::chrono::high_resolution_clock::now();
        double frame_time = 1.0 / fps;

        // Update playhead
        if (playbackMode == ofxSATimelineMode_RealTime_Absolute || playbackMode == ofxSATimelineMode_RealTime_Relative) {
            if(_relative) counters.playhead += frame_time * _frame;
            else counters.playhead = frame_time * _frame;
        }
        else {
            if(_relative) counters.playhead = frame_time * ( counters.frameNum + _frame);
            else counters.playhead = frame_time * _frame;
        }

        // Incremment rendered frames
        counters.frameCount++;

        // Remember (for rt modes)
        last_frame_time = start_time + std::chrono::seconds((long long)counters.playhead);

        checkLoops();

        updateInternals();

        onSeek.notify(counters);
    }
}

void ofxSATimeline::goToSeconds(double _seconds, bool _relative, bool _doPause){
    // Enable play, stay paused (allows setting frame)
    if(!playing && _doPause){
        pause();
    }

    // Set seconds
    if( _seconds < duration){
        if(_relative){
            counters.playhead += _seconds;
        }
        else {
            counters.playhead = _seconds;
        }

        // Incremment rendered frames
        counters.frameCount++;

        // Remember (for rt modes)
        if (playbackMode == ofxSATimelineMode_RealTime_Absolute)
            start_time = std::chrono::high_resolution_clock::now() - std::chrono::seconds((long long)(counters.playhead/playSpeed));
        else
            start_time = std::chrono::high_resolution_clock::now() - std::chrono::seconds((long long)counters.playhead);
        last_frame_time = start_time + std::chrono::seconds((long long)counters.playhead);

        checkLoops();

        updateInternals();

        onSeek.notify(counters);
    }
}

void ofxSATimeline::setLoop(ofxSATimelineLoopMode loopMode) {
    this->loopMode = loopMode;
}

void ofxSATimeline::setDuration(double duration) {
    this->duration = duration;
}

void ofxSATimeline::setDurationFromBeats(int bars, int noteValue) {
    ofxSATimeSignature timeSignature(bars, noteValue);
    double beatsInBar = static_cast<double>(timeSignature.beatsPerBar) * timeSignature.notesPerBeat;
    double beatsInMinute = static_cast<double>(timeSignature.bpm) / 60.0;
    setDuration(beatsInBar / beatsInMinute);
}

void ofxSATimeline::setPlaySpeed(double playSpeed) {
    this->playSpeed = playSpeed;
}

double ofxSATimeline::getPlaySpeed() const {
    return playSpeed;
}

// Optional, to update realtime modes in-between frames
void ofxSATimeline::tickUpdate(){
    // If not playing or paused, do nothing
    if (!playing || paused) {
        return;
    }

    tickPlayHead(false);
    checkLoops();
    updateInternals();
}

// At the begining of your frame, tick this. Todo: after frame so update() can prapare data for the next frame ?
void ofxSATimeline::tickFrame(){
    if(bStartNextFrame){
        start(startNextFrameOffet);
        bStartNextFrame = false;
        startNextFrameOffet = 0u;
        return;
    }

    // If not playing or paused, do nothing
    if (!playing || paused) {
        return;
    }

    tickPlayHead(true);
    checkLoops();
    updateInternals();

    onFrameTick.notify(counters);
}

// Updates the playhead and framecount to the most actual onces
void ofxSATimeline::tickPlayHead(const bool isNewFrame){
    // If not playing or paused, do nothing
    if (!playing || paused) {
        return;
    }

    // Incremment rendered frames
    if(isNewFrame) counters.frameCount++;

    auto current_time = std::chrono::high_resolution_clock::now();
    //auto elapsed_time = std::chrono::duration<double>(current_time - last_frame_time) * playSpeed;

    // Update playhead for rt modes
    if (playbackMode == ofxSATimelineMode_RealTime_Absolute || playbackMode == ofxSATimelineMode_RealTime_Relative) {

        //auto elapsed_time = std::chrono::duration<double>(current_time - ((playbackMode == ofxSATimelineMode_RealTime_Absolute)?start_time:last_frame_time)) * playSpeed;
        //playhead = elapsed_time.count();

        // TODO : rel/abs can be combined ! sum pausedTimes + use lastPausedTime.

        if (playbackMode == ofxSATimelineMode_RealTime_Absolute) {
            auto elapsed_time = std::chrono::duration<double>(current_time - start_time) * playSpeed;
            if(isNewFrame) counters.tDelta = elapsed_time.count(); // Tmp, fixme: when no newFrame, time is lost...
            if(playSpeed<0) counters.playhead = duration - std::abs(std::fmod(elapsed_time.count(), duration));
            else counters.playhead = elapsed_time.count();
            //playhead = glm::abs(glm::mod(playhead, duration));
            //std::cout << "elapsed=" << elapsed_time.count() << " Playhead=" << playhead << std::endl;
            //std::cout << "elapsed=" << glm::fract(1.0) << " Playhead=" << glm::mod(1.0,1.0) << std::endl;
        } else if (playbackMode == ofxSATimelineMode_RealTime_Relative) {
            auto elapsed_time = std::chrono::duration<double>(current_time - last_frame_time) * playSpeed;
            counters.playhead += elapsed_time.count();
        }
    }
    // Update playhead for offline mode
    else /*if (playbackMode == ofxSATimelineMode_Offline)*/ {
        if(isNewFrame){
            //double frame_time = 1.0 / (double)fps;
            counters.tDelta = 1.0 / (double)fps;
            // Add 1 frame's time to the playhead, will compute the framenum from that.
            counters.playhead = counters.tDelta * (double)(counters.frameNum+(1.0*playSpeed));
        }
    }

    // Remember (for rt modes)
    last_frame_time = current_time;
}

// Updates playhead for looping
void ofxSATimeline::checkLoops(){
    // Looping / Stopping
    // Check if duration exceeded
    if ( counters.playhead > duration || counters.playhead < 0) {
        // Handle looping
        switch(loopMode){
            case ofxSATimelineLoopMode::NoLoop :
                stop();
            break;
            case ofxSATimelineLoopMode::LoopOnce :
                if(counters.loopCount>0) stop();
            break;
            case ofxSATimelineLoopMode::LoopInfinite :

            break;
        }
        // Still playing ? Start over !
        if(playing){
            counters.loopCount++;

            // Reverse play: loop at end
            if(counters.playhead < 0){
                counters.playhead = duration;
            }
            else counters.playhead = std::fmod(counters.playhead, duration); // Fixme : Should not be fmod in offline mode

            onStart.notify(counters.loopCount);
        }
    }
}

// Updates internals and ramps according to playhead
void ofxSATimeline::updateInternals(){
    double frame_time = 1.0 / (double)fps;

    // Update counters
    counters.beatCount = std::floor(counters.playhead / timeSignature.getBeatDurationSecs());
    counters.noteCount = std::floor(counters.playhead / timeSignature.getNoteDurationSecs());
    counters.barCount  = std::floor(counters.playhead / timeSignature.getBarDurationSecs());
    if(playbackMode == ofxSATimelineMode_Offline) counters.frameNum = std::floor((counters.playhead) / frame_time + 0.00001); // Note: Some weird conversions happens sometimes here, printing not the same numbers below. floor / cast don't work, ceil seems to do fine.
    else counters.frameNum = std::floor(counters.playhead / frame_time); // Note: Some weird conversions happens sometimes here, printing not the same numbers below. floor / cast don't work, ceil seems to do fine.

    // Update ramps
    timeRamps.updateRamps(counters.playhead, timeSignature, duration);

    counters.progress = counters.playhead / duration;
}





double ofxSATimeline::getElapsedSeconds() const {
    //        auto current_time = std::chrono::high_resolution_clock::now();
    //        std::chrono::duration<double> elapsed_seconds = current_time - (start_time + paused_time);
    //        return elapsed_seconds.count();
    return counters.playhead;
}

unsigned int ofxSATimeline::getFrameNum() const {
    return static_cast<unsigned int>(counters.frameNum);
}

// Returns the number of rendered frames
unsigned int ofxSATimeline::getElapsedFrames() const {
    return counters.frameCount;
}

// Returns the number of frames if the composition is finite; 0 otherwise.
unsigned int ofxSATimeline::getTotalFrames() const {
    return duration * fps;
}

// Returns the number of bars if the composition is finite; 0 otherwise.
unsigned int ofxSATimeline::getTotalBars() const {
    //return ((float)duration/60.)*timeSignature.beatsPerBar;
    return glm::floor((float)duration / timeSignature.getBarDurationSecs());
}

// Returns the number of notes if the composition is finite; 0 otherwise.
unsigned int ofxSATimeline::getTotalNotes() const {
    return glm::floor((float)duration / timeSignature.getNoteDurationSecs());
}

double ofxSATimeline::getDuration() const {
    return duration;
}

// Playhead position
double ofxSATimeline::getProgress() const {
    return counters.progress;
    //return counters.playhead/getDuration();
    //return (double)frame_count/(double)getTotalFrames();
}

unsigned int ofxSATimeline::getFps() const {
    return fps;
}

void ofxSATimeline::setFps(unsigned int _fps) {
    fps = _fps;
}

//    double getCurrentBeat() const {
//        double beatsElapsed = getElapsedSeconds() * timeSignature.bpm / 60.0;
//        double beatsPerFrame = static_cast<double>(fps) / 60.0;
//        return std::fmod(beatsElapsed, timeSignature.notesPerBeat) + 1.0; // Beats are 1-indexed
//    }

bool ofxSATimeline::isPlaying() const {
    return playing;
}

ofxSATimelineMode ofxSATimeline::getPlayMode() const {
    return playbackMode;
}

void ofxSATimeline::setPlayMode(ofxSATimelineMode _mode) {
    playbackMode = _mode;
}

void ofxSATimeline::drawImGuiPlayControls(bool horizontalLayout){
    ImGui::PushID((const void *)this);

    static bool bShowSettings = false, bShowRamps = false;
    static double speedMin=-10, speedMax=10;

    // Show "tabs" / Settings toggle
    ImGui::BeginGroup();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    if(ImGui::Button("T##tab-time")){
        bShowSettings = false;
        bShowRamps = false;
    }
    if(ImGui::IsItemActive() || ImGui::IsItemHovered()){
        pos.x += 20;
        ImGui::GetForegroundDrawList()->AddRectFilled(pos, pos+ImVec2(35.f, ImGui::GetFrameHeight()), ImGui::GetColorU32(ImGuiCol_Button, 1.f));
        ImGui::GetForegroundDrawList()->AddText(pos+ImVec2(3,3), ImGui::GetColorU32(ImGuiCol_Text), "Time");
    }

    pos = ImGui::GetCursorScreenPos();
    if(ImGui::Button("S##tab-settings")){
        bShowSettings = true;
        bShowRamps = false;
    }
    if(ImGui::IsItemActive() || ImGui::IsItemHovered()){
        pos.x += 20;
        ImGui::GetForegroundDrawList()->AddRectFilled(pos, pos+ImVec2(58.f, ImGui::GetFrameHeight()), ImGui::GetColorU32(ImGuiCol_Button, 1.f));
        ImGui::GetForegroundDrawList()->AddText(pos+ImVec2(3,3), ImGui::GetColorU32(ImGuiCol_Text), "Settings");
    }

    pos = ImGui::GetCursorScreenPos();
    if(ImGui::Button("R##settings-ramps")){
        bShowSettings = false;
        bShowRamps = true;
    }
    if(ImGui::IsItemActive() || ImGui::IsItemHovered()){
        pos.x += 20;
        ImGui::GetForegroundDrawList()->AddRectFilled(pos, pos+ImVec2(42.f, ImGui::GetFrameHeight()), ImGui::GetColorU32(ImGuiCol_Button, 1.f));
        ImGui::GetForegroundDrawList()->AddText(pos+ImVec2(3,3), ImGui::GetColorU32(ImGuiCol_Text), "Ramps");
    }

    ImGui::EndGroup();

    if(horizontalLayout) ImGui::SameLine();

    if(bShowSettings){
        // Time signature
        ImGui::BeginGroup();

        // Time Signature
        ImGui::PushItemWidth(60);

        ImGui::SeparatorText("TimeSig");
        // Time Signature Info
        ImGui::SameLine();
    //            int posX = ImGui::GetCursorPosX();
    //            ImGui::SetCursorPosX(posX-10);
        if (ImGuiEx::BeginHelpMarker("[i]")){
            ImGui::SeparatorText("TS Info");
            ImGui::Text("1 beat = %.3fsec", timeSignature.getBeatDurationSecs());
            ImGui::Text("1 bar = %.3fsec", timeSignature.getBarDurationSecs());
            ImGui::Text("1 note = %.3fsec", timeSignature.getNoteDurationSecs());
            ImGuiEx::EndHelpMarker();
        }

        static unsigned int bpmMin = 1, bpmMax=512;
        if(ImGui::DragScalar("BPM", ImGuiDataType_U16, &timeSignature.bpm, 1.f, &bpmMin, &bpmMax, "%u")){
            timeSignature.updateValues();
        }
        static unsigned int noteStep = 1, noteStepFast=1;
        if(ImGui::InputScalar("Bars", ImGuiDataType_U16, &timeSignature.beatsPerBar, &noteStep, &noteStepFast, "%u")){
            timeSignature.updateValues();
        }
        if(ImGui::InputScalar("Notes", ImGuiDataType_U16, &timeSignature.notesPerBeat, &noteStep, &noteStepFast, "%u")){
            timeSignature.updateValues();
        }
        ImGui::PopItemWidth();
        ImGui::EndGroup();

        if(horizontalLayout) {
            ImGui::SameLine();
            ImGui::Dummy({8,8});
            ImGui::SameLine();
        }

        // Playback
        ImGui::BeginGroup();
        if(horizontalLayout) ImGui::PushItemWidth(80);
        ImGui::SeparatorText("Playback");

        if(ImGui::InputScalar("Fps", ImGuiDataType_U16, &fps, &noteStep, &noteStepFast, "%u")){
            //timeSignature.updateValues();
        }

        // Playback
        static int currentPBMode=-1;
        currentPBMode = playbackMode==ofxSATimelineMode::ofxSATimelineMode_RealTime_Absolute?0:(playbackMode==ofxSATimelineMode::ofxSATimelineMode_RealTime_Relative?1:2);
        if(ImGui::Combo("Playback", &currentPBMode, "Real-time (absolute)\0Real-time (relative)\0Offline\0\0")){
            playbackMode = currentPBMode==0?ofxSATimelineMode::ofxSATimelineMode_RealTime_Absolute : (currentPBMode==1?ofxSATimelineMode::ofxSATimelineMode_RealTime_Relative : ofxSATimelineMode::ofxSATimelineMode_Offline);
        }

        // Looping
        static int currentLoopMode=-1;
        currentLoopMode = loopMode==ofxSATimelineLoopMode::NoLoop?0:(loopMode==ofxSATimelineLoopMode::LoopOnce?1:2);
        if(ImGui::Combo("Loop", &currentLoopMode, "No Loop\0Loop Once\0Loop Infinite\0\0")){
            loopMode = currentLoopMode==0?ofxSATimelineLoopMode::NoLoop:(currentLoopMode==1?ofxSATimelineLoopMode::LoopOnce:ofxSATimelineLoopMode::LoopInfinite);
        }

        // Play speed
        ImGui::DragScalar("Speed", ImGuiDataType_Double, &playSpeed, 0.005f, &speedMin, &speedMax, "%7.3f");

        if(horizontalLayout) ImGui::PopItemWidth();
        ImGui::EndGroup();

        if(horizontalLayout) ImGui::SameLine();

        // Duration
        ImGui::BeginGroup();
        if(horizontalLayout) ImGui::PushItemWidth(80);
        ImGui::SeparatorText("Duration");

        unsigned int durationHours = glm::floor(duration/(3600));
        unsigned int durationMinutes = glm::mod(glm::floor(duration/60), 60.);
        unsigned int durationSeconds = glm::mod(duration, 60.);
        static unsigned int durationMin = 0, durationHoursMax = 23, durationMinSecsMax=59;
        bool updateDurations = false;
        if(ImGui::DragScalar("##hours", ImGuiDataType_U16, &durationHours, 1.f, &durationMin, &durationHoursMax, "%u hours")){
            updateDurations = true;
        }
        if(ImGui::DragScalar("##minutes", ImGuiDataType_U16, &durationMinutes, 1.f, &durationMin, &durationMinSecsMax, "%u mins")){
            updateDurations = true;
        }
        if(ImGui::DragScalar("##seconds", ImGuiDataType_U16, &durationSeconds, 1.f, &durationMin, &durationMinSecsMax, "%u secs")){
            updateDurations = true;
        }

        if(updateDurations){
            duration = glm::max(durationHours*3600 + durationMinutes*60 + durationSeconds, 1u);
        }
        if(horizontalLayout) ImGui::PopItemWidth();
        ImGui::EndGroup();

        if(horizontalLayout) ImGui::SameLine();

        // Duration info
        ImGui::BeginGroup();
        //if(horizontalLayout) ImGui::PushItemWidth(80);
        ImGui::SeparatorText("Info");

        ImGui::Text("Duration: %.0f sec", duration);
        ImGui::Text("%u frames total", getTotalFrames());
        ImGui::Text("x bars (measures)");
        ImGui::Text("x notes");

        //ImGui::Spacing();
        //if(horizontalLayout) ImGui::PopItemWidth();
        ImGui::EndGroup();

        //if(horizontalLayout) ImGui::SameLine();
    } // End settings

    // Ramps
    else if(bShowRamps){
//        for(unsigned int i=0; i < ofxSATL_Ramp_Hist_Size-1; i++){
//            beatProgressHist[i] = beatProgressHist[i+1];
//        }
//        beatProgressHist[ofxSATL_Ramp_Hist_Size-1] = timeRamps.beatProgress;

        // Signals & Ramps
        ImGui::BeginGroup();

        // Bar Ramps
        static float barProgressHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Bar progress: %.2f", barProgressHist, timeRamps.barProgress, !paused);
        static float barPulseHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Bar pulse: %.2f", barPulseHist, timeRamps.barPulse, !paused);
        static float barStepHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Bar Step: %.2f", barStepHist, timeRamps.barStep, !paused);

        ImGui::Separator();

        // Beat Ramps
        static float beatProgressHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Beat Progress: %.2f", beatProgressHist, timeRamps.beatProgress, !paused);
        static float beatPulseHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Beat Pulse: %.2f", beatPulseHist, timeRamps.beatPulse, !paused);
        static float beatStepHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Beat Step: %.2f", beatStepHist, timeRamps.beatStep, !paused);

        ImGui::Separator();

        // Note Ramps
        static float noteProgressHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Note Progress: %.2f", noteProgressHist, timeRamps.noteProgress, !paused);
        static float notePulseHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Note Pulse: %.2f", notePulseHist, timeRamps.notePulse, !paused);
        static float noteStepHist[ofxSATL_Ramp_Hist_Size] = {0};
        ImGuiEx::RampGraph("Note Step: %.2f", noteStepHist, timeRamps.noteStep, !paused);

        ImGui::EndGroup(); // End Ramps group
    } // end ramps section

    // Timer clock & Controls
    else {

        // TimeSig
        ImGui::BeginGroup();
        ImGui::PushItemWidth(100);
        //ImGui::BeginGroup();
        //ImGui::SeparatorText("TimeSig");

        ImGui::BeginDisabled();
        //ImGui::Dummy({100,20});
        ImGui::BeginGroup();
        ImGui::Text("Bar");
        ImGui::VSliderFloat("##barprogress", ImVec2(14, 50), &timeRamps.barProgress, 0.0f, 1.0f, "");
        ImGui::SameLine();
        ImGui::VSliderFloat("##barstep", ImVec2(6, 50), &timeRamps.barStep, 0.0f, 1.0f, "");
        ImGui::Text("%04lu", counters.barCount%10000);
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Beat");
        ImGui::VSliderFloat("##beatprogress", ImVec2(14, 50), &timeRamps.beatProgress, 0.0f, 1.0f, "");
        ImGui::SameLine();
        ImGui::VSliderFloat("##beatstep", ImVec2(6, 50), &timeRamps.beatStep, 0.0f, 1.0f, "");
        ImGui::Text("%04lu", counters.beatCount%10000);
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Note");
        ImGui::VSliderFloat("##noteprogress", ImVec2(14, 50), &timeRamps.noteProgress, 0.0f, 1.0f, "");
        ImGui::SameLine();
        ImGui::VSliderFloat("##notestep", ImVec2(6, 50), &timeRamps.noteStep, 0.0f, 1.0f, "");
        ImGui::Text("%04lu", counters.noteCount%10000);
        ImGui::EndGroup();

        ImGui::EndDisabled();
        //ImGui::EndGroup();
        ImGui::PopItemWidth();
        ImGui::EndGroup();

        if(horizontalLayout) ImGui::SameLine();

        // Timers
        ImGui::BeginGroup();

        ImGui::BeginDisabled();

        if(horizontalLayout){
            ImGui::Dummy({30,5});
            ImGui::SameLine();
        }
        ImGui::Text("Time");
        ImGui::EndDisabled();

        static bool bShowSecsWithMins = false;
        double elapsed = getElapsedSeconds();
        if(bShowSecsWithMins){
            //unsigned int durationHours = glm::floor(elapsed/(3600));
            unsigned int durationMinutes = glm::floor(glm::mod(glm::floor(elapsed/60), 60.));
            unsigned int durationSeconds = glm::floor(glm::mod(elapsed, 60.));
            unsigned int durationMs = glm::round(glm::mod(elapsed, 1.)*1000);
            ImGui::Text(" %02u:%02u:%03u s", durationMinutes, durationSeconds, durationMs);
        }
        else ImGui::Text("%10.4f s", elapsed);
        // Toggle display style on click
        if(ImGui::IsItemClicked()){
            bShowSecsWithMins = !bShowSecsWithMins;
        }
        ImGui::Text("%10u f", getFrameNum());
        ImGui::Text("%4lu.%2lu.%2lu tc", counters.barCount, counters.beatCount%timeSignature.beatsPerBar, counters.noteCount%timeSignature.notesPerBeat);
        ImGui::Text("%10.6f %%", getProgress());
        //if(counters.loopCount>0) ImGui::Text("Loops   : %10lu", counters.loopCount);

        //

        ImGui::EndGroup(); // Timers

        if(horizontalLayout){
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
        }

        // Time
        ImGui::BeginGroup();
        //ImGui::SeparatorText("Time");
        static double pHeadMin=0.;
        ImVec2 barSize = {ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()-2};
        ImGui::Dummy({barSize.x, 8});
        ImVec2 barPos = ImGui::GetCursorScreenPos();
        double barScaleSecond = barSize.x / duration;
        barPos.y+=1;
        ImGui::SetNextItemWidth(barSize.x);
        if(ImGui::SliderScalar("##Playhead",ImGuiDataType_Double, &counters.playhead, &pHeadMin, &duration, "" )){
            goToSeconds(counters.playhead);
        }

        ImDrawList* wdl = ImGui::GetWindowDrawList();
        //wdl->AddRect(barPos, barPos+barSize, IM_COL32(255,255,255,255));

        // Draw Bars
        unsigned int numBars = glm::floor(duration / timeSignature.getBarDurationSecs());
        for(unsigned int bi=0; bi < numBars; bi++){
            float posX = bi * timeSignature.getBarDurationSecs() * barScaleSecond;
            wdl->AddLine({barPos.x+posX, barPos.y}, {barPos.x+posX, barPos.y+barSize.y}, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        }

        // Draw Beats
        unsigned int numBeats = glm::floor(duration / timeSignature.getBeatDurationSecs());
        for(unsigned int bi=0; bi < numBeats; bi++){
            float posX = bi * timeSignature.getBeatDurationSecs() * barScaleSecond;
            wdl->AddLine({barPos.x+posX, barPos.y}, {barPos.x+posX, barPos.y+barSize.y}, ImGui::GetColorU32(ImGuiCol_TextDisabled, .2f));
        }

        // Draw seconds
        int fontPixels = ImGui::GetFontSize();
        double tScale = 10.0; // By default, every 10 secs
        unsigned int maxCharSlots = barSize.x/(fontPixels*3);
        unsigned int numTimeSlots = duration/tScale;
        if(numTimeSlots > maxCharSlots){
            tScale = 30.0;
        }
        for(float t = 0; t <= duration; t+=tScale){
            float posX = t * barScaleSecond;
            std::string label = ofToString(glm::floor(t/60.f)) + ":" + ofToString(glm::mod(t,60.f));
            wdl->AddLine({barPos.x+posX, barPos.y}, {barPos.x+posX, barPos.y+barSize.y}, ImGui::GetColorU32(ImGuiCol_Text));
            wdl->AddText({barPos.x+posX-fontPixels*1.f, barPos.y-barSize.y}, ImGui::GetColorU32(ImGuiCol_Text, .6f), label.c_str());
        }

        // Big cursor line
        float playHeadX = getProgress() * barSize.x;
        wdl->AddLine({barPos.x+playHeadX, barPos.y-4}, {barPos.x+playHeadX, barPos.y+barSize.y+4}, ImGui::GetColorU32(ImGuiCol_ButtonActive));


        // Play Controls
        ImGui::BeginGroup();

        // Start over
        if(ImGui::Button("<-")){
            goToFrame(0);
        }
        ImGui::SameLine();

        // Start/Stop
        if(!playing){
            if(ImGui::Button("Play")){
                start();
            }
        }
        else {
            if(ImGui::Button("Stop")){
                stop();
            }
        }
        ImGui::SameLine();

        // Pause / Resume
        if(playing && !paused){
            if(ImGui::Button("Pause ")){
                pause();
            }
        }
        else if(ImGui::Button("Resume")){
            if(!playing) start();
            else resume();
        }
        ImGui::SameLine();

        // Prev-next controls
        ImGui::Spacing();
        ImGui::SameLine();

        bool endDisabled = false;
        if(!(paused || playSpeed==0)){
            ImGui::BeginDisabled();
            endDisabled = true;
        }
        ImGuiDir frameNav = ImGuiEx::ButtonPair(ImGuiDir_Left, ImGuiDir_Right);
        if(frameNav!=ImGuiDir_None){
            if(frameNav==ImGuiDir_Left) nextFrame(ImGui::IsKeyDown(ImGuiMod_Shift)?(-1*fps):-1);
            else nextFrame(ImGui::IsKeyDown(ImGuiMod_Shift)?fps:1);
        }
        if(endDisabled) ImGui::EndDisabled();

        if(horizontalLayout) ImGui::SameLine();

        // Speed
        ImGui::Text(" ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(50);
        ImGui::DragScalar("##playSpeed", ImGuiDataType_Double, &playSpeed, 0.001f, &speedMin, &speedMax, "%6.3f" );
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_ForTooltip) && !ImGui::IsItemActive() && ImGui::BeginItemTooltip()){
            ImGui::Text("Play Speed: %.001f", playSpeed);
            ImGui::EndTooltip();
        }

        ImGui::SameLine();

        // Loop toggle
        if(loopMode==ofxSATimelineLoopMode::NoLoop){
            if(ImGui::Button("x##noloop")){
                loopMode=ofxSATimelineLoopMode::LoopOnce;
            }
        }
        else if(loopMode==ofxSATimelineLoopMode::LoopOnce){
            if(ImGui::Button("|##looponce")){
                loopMode=ofxSATimelineLoopMode::LoopInfinite;
            }
        }
        else {
            if(ImGui::Button("o##loop")){
                loopMode=ofxSATimelineLoopMode::NoLoop;
            }
        }
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_ForTooltip) && !ImGui::IsItemActive() && ImGui::BeginItemTooltip()){
            ImGui::Text("Looping: %s", loopMode==ofxSATimelineLoopMode::LoopOnce?"Once":(loopMode==ofxSATimelineLoopMode::NoLoop?"No Loop":"Loop Infinite"));
            ImGui::EndTooltip();
        }
        ImGui::SameLine();
        ImGui::Text("%03lu", counters.loopCount);

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        // Playback Mode toggle
        if(playbackMode==ofxSATimelineMode::ofxSATimelineMode_Offline){
            if(ImGui::Button("offline##bpmo")){
                playbackMode=ofxSATimelineMode::ofxSATimelineMode_RealTime_Absolute;
            }
        }
        else if(playbackMode==ofxSATimelineMode::ofxSATimelineMode_RealTime_Absolute){
            if(ImGui::Button("r-t Abs##pbmra")){
                playbackMode=ofxSATimelineMode::ofxSATimelineMode_RealTime_Relative;
            }
        }
        else{
            if(ImGui::Button("r-t Rel##pbmrr")){
                playbackMode=ofxSATimelineMode::ofxSATimelineMode_Offline;
            }
        }
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_ForTooltip) && !ImGui::IsItemActive() && ImGui::BeginItemTooltip()){
            ImGui::Text("Playback Mode: %s", playbackMode==ofxSATimelineMode::ofxSATimelineMode_Offline?"Offline":(playbackMode==ofxSATimelineMode::ofxSATimelineMode_RealTime_Absolute?"Real Time (absolute)":"Real Time (relative)"));
            ImGui::EndTooltip();
        }

        ImGui::EndGroup(); // Controls
        ImGui::EndGroup(); // time section
    }

    ImGui::PopID();
}

void ofxSATimeline::drawImGuiTimelineWindow(bool* p_open){
    if(ImGui::Begin("Timeline", p_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)){
        drawImGuiPlayControls();
    }

    ImGui::End();
}

void ofxSATimeline::reset() {
    playing = false;
    paused = false;
    counters.playhead = 0.0;
    counters.frameCount = 0;
    counters.frameNum = 0;
    counters.loopCount = 0;
    counters.noteCount = 0;
    counters.beatCount = 0;
    counters.barCount = 0;
    counters.progress = 0;

    //loop_complete = false;
    start_time = std::chrono::high_resolution_clock::now();
    paused_time = std::chrono::duration<double>(0);

    updateInternals();
    //timeRamps.updateRamps(counters.playhead, timeSignature, duration);
}

// Load + Save
#if ofxSA_XML_ENGINE == ofxSA_XML_ENGINE_PUGIXML
bool ofxSATimeline::populateXmlNode(pugi::xml_node &_node){

    // FPS
    _node.append_child("fps").text().set(fps);

    // Duration
    _node.append_child("duration").text().set(duration);

    // LoopMode
    _node.append_child("loop_mode").text().set(loopMode);

    // PlaybackMode
    _node.append_child("play_mode").text().set(playbackMode);

    // Timesignature
    pugi::xml_node tsNode = _node.append_child("time_signature");
    tsNode.append_attribute("bpm").set_value(timeSignature.bpm);
    tsNode.append_attribute("beats_per_bar").set_value(timeSignature.beatsPerBar);
    tsNode.append_attribute("notes_per_beat").set_value(timeSignature.notesPerBeat);

    // Todo: PlaySpeed ?

    return true;
}

bool ofxSATimeline::retrieveXmlNode(pugi::xml_node &_node){
    bool ret = true;

    // FPS
    if(pugi::xml_node fpsNode = _node.child("fps")){
        fps = fpsNode.text().as_uint();
    }
    else ret = false;

    // Duration
    if(pugi::xml_node dNode = _node.child("duration")){
        duration = dNode.text().as_double();
    }
    else ret = false;

    // LoopMode
    if(pugi::xml_node lNode = _node.child("loop_mode")){
        loopMode = static_cast<ofxSATimelineLoopMode>(lNode.text().as_int());
    }
    else ret = false;

    // PlaybackMode
    if(pugi::xml_node fpsNode = _node.child("play_mode")){
        playbackMode = static_cast<ofxSATimelineMode>(fpsNode.text().as_uint());
    }
    else ret = false;

    // Timesignature
    if(pugi::xml_node tsNode = _node.child("time_signature")){
        timeSignature.bpm = tsNode.attribute("bpm").as_uint();
        timeSignature.beatsPerBar = tsNode.attribute("beats_per_bar").as_uint();
        timeSignature.notesPerBeat = tsNode.attribute("notes_per_beat").as_uint();
    }
    else ret = false;

    reset();

    return ret;
}
#endif

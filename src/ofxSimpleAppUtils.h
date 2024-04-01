#pragma once

#include "ofxSimpleAppConfig.h"
#include "ofAppRunner.h"
#include <iostream>
#include "ofMath.h"
//#include "ofGraphics.h"
#include "ofxImGui.h"
#include "ofxImGuiLoggerChannel.h"

//void ofDrawBitMapStringHighlight(const std::string& _string, const ofRectangle& _rect){
//    ofDrawBitmapStringHighlight(_string, _rect.getTopLeft(),)
//}

//template<class ofAppClass>
//int ofxSimpleAppGenericMain();


// You can use this main() function to quickly setup your app, or write your own one for more advanced usages.
// Cannot separate definition and implementation because we cannot instantiate a template function womewhere else, and the ofApp class is unknown here. So we put the whole definition here and let users instantiate their classes on call.
template<class ofAppClass>
int ofxSimpleAppGenericMain(){

	// Create window
#if defined( TARGET_OPENGLES ) || defined( FORCE_GLES )
	ofGLESWindowSettings settings;
	settings.setGLESVersion(ofxSA_GLES_VERSION);
#else
	ofGLWindowSettings settings;
	settings.setGLVersion(ofxSA_GL_VERSION_MAJ, ofxSA_GL_VERSION_MIN);
#endif
	settings.setSize(ofxSA_WINDOW_WIDTH, ofxSA_WINDOW_HEIGHT);
	settings.title = ofxSA_APP_NAME;
	auto window = ofCreateWindow(settings);

	// Run App
	auto app = std::make_shared<ofAppClass>();
	ofRunApp(window, app);
	ofRunMainLoop();

	return 0;
}

template<typename ARRAY_T, typename VALUE_T>
void syncHistogram(ARRAY_T(&_histogram)[ofxSA_FPS_HISTORY_SIZE], const VALUE_T& _newValue) {
	for(int i=0; i<ofxSA_FPS_HISTORY_SIZE-1; ++i){
		_histogram[i]=_histogram[i+1];
	}
	_histogram[ofxSA_FPS_HISTORY_SIZE-1] = (ARRAY_T)_newValue;
}

std::ostream& operator<< (std::ostream& _stream, const glm::uvec2& _vec2);

template<class T, size_t N>
constexpr size_t array_size(T (&)[N]);

// Idle timer
struct IdleTimer {
    public:
        IdleTimer(){
            notIdleHeartBeat();
        }

        void notIdleHeartBeat(){
            lastActiveTime = ofGetElapsedTimef();
        }

        float getElapsed(){
            return ofGetElapsedTimef() - lastActiveTime;
        }

    private:
        float lastActiveTime = 0.f;
};

// C+ smoothstep implementation
// Todo: Unused ?
//#include <math.h>
//#include <cmath>
//#include <algorithm>
template<typename TYPE>
TYPE smoothstep(TYPE edge0, TYPE edge1, TYPE x);

// ImGui Extensions
namespace ImGuiEx {

    // Helpmarkers (similar to ImGuiDemo code
    void ShowHelpMarker(const char* desc);
    bool BeginHelpMarker(const char* marker); // Call EndHelpMarker() if true, after submitting toltip content.
    void EndHelpMarker();
    bool ButtonActive(const char* id, bool isActive = false);

    // Toolbar stuff
    bool BeginToolBar(bool verticalLayout=true);
    void EndToolBar();
    bool ToolBarItem(const char* id, bool isActive=false);
    bool ToolBarItem(const char* id, bool* isActive);

    // Menu shortcut handler helper
    // From https://github.com/ocornut/imgui/issues/7081#issuecomment-1857951212
    // - - - -
    // Wrapper for menu that can be opened with a global shortcut
    // or submenu with a local shortcut
    // usage: if (ImGui::BeginMenu("File", ImGuiMod_Alt | ImGuiKey_F)) {...}
    inline bool BeginMenu(const char* label, const ImGuiKeyChord key);
    // Wrapper for menuitem that can be opened with a local shortcut
    // Usage: if (ImGui::MenuItem("Global Exit", "Alt+X")){...}
    inline bool MenuItem(const char* label, const ImGuiKeyChord key);

    template<typename LoggerBuffer=ImGuiTextBuffer>
    void DrawLoggerChannelClear(LoggerBuffer& , ImVector<int>& );

    template<typename LoggerBuffer>
    void DrawLoggerChannelClear(ImGuiTextBuffer& buf, ImVector<int>& LineOffsets);

    // Default trap with compile-time error if used.
    template<typename LoggerChannelType=ofxImGui::LoggerChannel>
    static typename std::enable_if<!std::is_base_of<ofxImGui::LoggerChannel, LoggerChannelType>::value, void>::type DrawLoggerChannel(const char* title, LoggerChannelType& logChannel, bool* p_open = NULL ){
        static_assert(true, "DrawLoggerChannel needs to be derived from ofxImGui::LoggerChannel !");
    }

    // Anything const / non-const ofxImGui::LoggerChannel or derived from it.
    template<typename LoggerChannelType=ofxImGui::LoggerChannel>
    static typename std::enable_if< std::is_base_of<ofxImGui::LoggerChannel, LoggerChannelType>::value, void>::type DrawLoggerChannel(const char* title, LoggerChannelType& logChannel, bool* p_open = NULL ){
        using ImGuiTextBufferCondOrNot = typename std::conditional< std::is_const<LoggerChannelType>::value, const ImGuiTextBuffer, ImGuiTextBuffer>::type;
        //const bool isConst = std::is_const<LoggerChannelType>::value;
        //std::cout << "logChannel in imguiex=" << &logChannel << std::endl;
        // Code is almost a replica of ExampleAppLog, variable names are changed, other changes are commented
        // See https://github.com/ocornut/imgui/blob/65c4c0a490f64e7929a63c43be05748b34ec6228/imgui_demo.cpp#L6893-L6979
        static bool AutoScroll;
        static ImGuiTextFilter Filter;
        static ImVector<int> LineOffsets;
        if(LineOffsets.size()<=0) LineOffsets.push_back(0); // Fixes empty log (see original clear() function)
        ImGuiTextBufferCondOrNot& Buf = logChannel.getBuffer();

        if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_NoCollapse)){ // Change: added flag
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options")) {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");

        DrawLoggerChannelClear(Buf, LineOffsets); // Change : Replaced sameline & button by this func

        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        // Change: Remove whole clear condition + scope
        if (copy)
            ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = Buf.begin();
        const char* buf_end = Buf.end();
        if (Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of
            // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
            // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
            // within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
            // on your side is recommended. Using ImGuiListClipper requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display
            // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
            // it possible (and would be recommended if you want to search through tens of thousands of entries).
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }

    // Bridge for shared pointers
    template<typename LoggerChannelType=ofxImGui::LoggerChannel>
    static void DrawLoggerChannel(const char* title, std::shared_ptr<LoggerChannelType>& logChannel, bool* p_open = NULL ){
        using ImGuiLogChannelCondOrNot = typename std::conditional< std::is_const<LoggerChannelType>::value, const ofxImGui::LoggerChannel, ofxImGui::LoggerChannel>::type;
        using ImGuiTextBufferCondOrNot = typename std::conditional< std::is_const<LoggerChannelType>::value, const ImGuiTextBuffer, ImGuiTextBuffer>::type;

        if(!logChannel) return;
        ImGuiLogChannelCondOrNot& lc = *logChannel;

        DrawLoggerChannel(title, lc, p_open);
    }

}

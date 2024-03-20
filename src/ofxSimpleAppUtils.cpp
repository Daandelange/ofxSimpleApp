

#include "ofxSimpleAppUtils.h"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"

#include "imgui_internal.h"

//void ofDrawBitMapStringHighlight(const std::string& _string, const ofRectangle& _rect){
//    ofDrawBitmapStringHighlight(_string, _rect.getTopLeft(),)
//}
//using namespace std;
//using namespace glm;

std::ostream& operator << (std::ostream& _stream, const glm::uvec2& _vec2){
    _stream << glm::to_string(_vec2) << "]";
    return _stream;
}

// Helper to get size of a fixed array
template<class T, size_t N>
constexpr size_t array_size(T (&)[N]) { return N; }

// C+ smoothstep implementation
// From : https://stackoverflow.com/questions/28889210/smoothstep-function
// Imprved to match all types by Daan
template<typename TYPE>
float smoothstep(TYPE edge0, TYPE edge1, TYPE x){
    // Scale, bias and saturate x to 0..1 range
    float y = ofClamp(((float)x - edge0) / ((float)edge1 - edge0), 0.0f, 1.0f);
    // Note: std::clamp will be available in c++17
    // Evaluate polynomial
    return y * y * ( y * (3 - 2));
}

// From imgui_demo.cpp
namespace ImGuiEx {
    // Help marker
    bool BeginHelpMarker(const char* marker = "[?]"){
        ImGui::SameLine();
        ImGui::TextDisabled("%s", marker);
        if (ImGui::IsItemHovered()) {
            if (ImGui::BeginTooltip()) {
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                return true;
            }
        }
        return false;
    }

    void ShowHelpMarker(const char* desc){
        if (BeginHelpMarker()){
            ImGui::TextUnformatted(desc);
            EndHelpMarker();
        }
    }

    void EndHelpMarker(){
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }

    bool ButtonActive(const char* id, bool isActive){
        if(isActive){
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TabActive));
        }
        bool ret = ImGui::Button(id);
        if(isActive){
            ImGui::PopStyleColor();
        }
        return ret;
    }

    // Toolbar functions

    // Mix of ImGui::BeginMainMenuBar() and XXX
    bool BeginToolBar(bool verticalLayout){
        ImGuiContext& g = *GImGui;
        ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();

        // Notify of viewport change so GetFrameHeight() can be accurate in case of DPI change
        ImGui::SetCurrentViewport(NULL, viewport);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;// | ImGuiWindowFlags_MenuBar;

        float height = ImGui::GetFrameHeight() + (verticalLayout?g.Style.WindowPadding.y:g.Style.WindowPadding.x);//ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f)*2;
        bool is_open = ImGui::BeginViewportSideBar("##MainToolBar", viewport, verticalLayout?ImGuiDir_Left:ImGuiDir_Up, height, window_flags);


        if (is_open)
            ImGui::Begin("##MainToolBar");
        else
            ImGui::End();

        return is_open;
    }
    void EndToolBar(){
        ImGui::End();


        ImGui::End();
    }
    bool ToolBarItem(const char* id, bool isActive){
        static int tmp;
        std::string btnId = id;
        btnId = btnId.substr(0,1);
        btnId.append("##toolbar-item-");
        btnId.append(id);
        ImVec2 pos = ImGui::GetCursorScreenPos();

        if(isActive){
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TabActive));
        }

        bool ret = false;
        if(ImGui::Button(btnId.c_str()))
            ret=true;

        if(ret) std::cout << "ret = " << (ret?1:0) << "\tbtnId = " << btnId << std::endl;
        if(isActive){
            ImGui::PopStyleColor();
        }

        if(ImGui::IsItemActive() || ImGui::IsItemHovered()){
            pos.x += 20;
            ImGui::GetForegroundDrawList()->AddRectFilled(pos, pos+ImVec2(58.f, ImGui::GetFrameHeight()), ImGui::GetColorU32(ImGuiCol_Button, 1.f));
            ImGui::GetForegroundDrawList()->AddText(pos+ImVec2(3,3), ImGui::GetColorU32(ImGuiCol_Text), id);
        }
        return ret;
    }

    template<typename LoggerBuffer>
    void DrawLoggerChannelClear(LoggerBuffer& , ImVector<int>& ){
        // Do nothing by default. Any const variables will disable this functionality
    }
    // Enable for any non-const derived from ofxImGui::LoggerChannel
//    template<typename LoggerChannelType>
//    typename enable_if<std::is_base_of<ofxImGui::LoggerChannel, LoggerChannelType>::value && !std::is_const<LoggerChannelType>::value, void>::type DrawLoggerChannelClear(LoggerChannelType& ch, ImVector<int>& LineOffsets){
//        // Button
//        ImGui::SameLine();
//        if(ImGui::Button("Clear")){
//            ch.getBuffer().clear();
//            LineOffsets.clear();
//            LineOffsets.push_back(0);
//        }
//    }
//    template<typename LoggerBuffer>
//    typename enable_if<std::is_base_of<ImGuiTextBuffer, LoggerBuffer>::value && !std::is_const<LoggerBuffer>::value, void>::type DrawLoggerChannelClear(LoggerChannelType& ch, ImVector<int>& LineOffsets){
//        // Button
//        ImGui::SameLine();
//        if(ImGui::Button("Clear")){
//            ch.getBuffer().clear();
//            LineOffsets.clear();
//            LineOffsets.push_back(0);
//        }
//    }
    template<>
    void DrawLoggerChannelClear(ImGuiTextBuffer& buf, ImVector<int>& LineOffsets){
        // Button
        ImGui::SameLine();
        if(ImGui::Button("Clear")){
            buf.clear();
            LineOffsets.clear();
            LineOffsets.push_back(0);
        }
    }
    // ofBaseLogger helper
    //static void DrawLoggerChannel(const char* title, ofBaseLoggerChannel& logChannel, bool* p_open = NULL ){
   //template<typename LoggerChannelType=ofxImGui::LoggerChannel>
//    template<typename LoggerChannelType=ofBaseLoggerChannel>
//    static void DrawLoggerChannelImpl(const char* title, LoggerChannelType& logChannel, bool* p_open = NULL ){
//        DrawLoggerChannelImpl(title,logChannel, p_open);
//    }

    //template<typename LoggerChannelType=ofxImGui::LoggerChannel>
    //static typename enable_if<std::is_base_of<ofxImGui::LoggerChannel, LoggerChannelType>::value, void>::type DrawLoggerChannel(const char* title, LoggerChannelType& logChannel, bool* p_open = NULL ){
    // Mad template code to allow calling the same function with const & non-const logger channel (adds clear functionality)
#ifdef NEVAHDEF
    template<typename LoggerChannelType>
    typename std::enable_if<!std::is_base_of<ofxImGui::LoggerChannel, LoggerChannelType>::value, void>::type DrawLoggerChannel(const char*, LoggerChannelType&, bool* ){
        static_assert(true, "DrawLoggerChannel needs to be derived from ofxImGui::LoggerChannel !");
    }
    template<typename LoggerChannelType>
    typename std::enable_if<std::is_base_of<ofxImGui::LoggerChannel, LoggerChannelType>::value, void>::type DrawLoggerChannel(const char* title, LoggerChannelType& logChannel, bool* p_open ){

        using ImGuiTextBufferCondOrNot = typename std::conditional< std::is_const<LoggerChannelType>::value, const ImGuiTextBuffer, ImGuiTextBuffer>::type;
        //const bool isConst = std::is_const<LoggerChannelType>::value;

        // Code is almost a replica of ExampleAppLog, variable names are changed, other changes are commented
        // See https://github.com/ocornut/imgui/blob/65c4c0a490f64e7929a63c43be05748b34ec6228/imgui_demo.cpp#L6893-L6979
        static bool AutoScroll;
        static ImGuiTextFilter Filter;
        static ImVector<int> LineOffsets;
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
    };

    // Bridge shared ptr to ref
    template<typename LoggerChannelType>
    void DrawLoggerChannel(const char* title, std::shared_ptr<LoggerChannelType>& logChannel, bool* p_open ){

        using ImGuiLogChannelCondOrNot = typename std::conditional< std::is_const<LoggerChannelType>::value, const ofxImGui::LoggerChannel, ofxImGui::LoggerChannel>::type;

        if(!logChannel) return;
        ImGuiLogChannelCondOrNot& lc = *logChannel;
        DrawLoggerChannel(title, lc, p_open);
    }
#endif
//    template<typename LoggerChannelType=ofxImGui::LoggerChannel>
//    static typename enable_if<std::is_base_of<ofxImGui::LoggerChannel, typename std::pointer_traits<LoggerChannelType>::element_type >::value, void>::type DrawLoggerChannel(const char* title, LoggerChannelType& logChannel, bool* p_open = NULL ){
//        if(!logChannel) return;
//        ofxImGui::LoggerChannel& lc = *logChannel;
//        DrawLoggerChannel(title, lc, p_open);
//    }
}


// UnrealTheme from dev0-1
// Source: https://github.com/Patitotective/ImThemes/blob/b57225e884c4da9e42b8789e98df2e433dd2be7d/themes.toml#L640-L730

#pragma once

#include "BaseTheme.h"

class UnrealTheme : public ofxImGui::BaseTheme {

    void setup() override {
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0;
        style.DisabledAlpha = 0.6000000238418579;
        style.WindowPadding = ImVec2(8.0, 8.0);
        style.WindowRounding = 0.0;
        style.WindowBorderSize = 1.0;
        style.WindowMinSize = ImVec2(32.0, 32.0);
        style.WindowTitleAlign = ImVec2(0.0, 0.5);
        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ChildRounding = 0.0;
        style.ChildBorderSize = 1.0;
        style.PopupRounding = 0.0;
        style.PopupBorderSize = 1.0;
        style.FramePadding = ImVec2(4.0, 3.0);
        style.FrameRounding = 0.0;
        style.FrameBorderSize = 0.0;
        style.ItemSpacing = ImVec2(8.0, 4.0);
        style.ItemInnerSpacing = ImVec2(4.0, 4.0);
        style.CellPadding = ImVec2(4.0, 2.0);
        style.IndentSpacing = 21.0;
        style.ColumnsMinSpacing = 6.0;
        style.ScrollbarSize = 14.0;
        style.ScrollbarRounding = 9.0;
        style.GrabMinSize = 10.0;
        style.GrabRounding = 0.0;
        style.TabRounding = 4.0;
        style.TabBorderSize = 0.0;
        style.TabMinWidthForCloseButton = 0.0;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5, 0.5);
        style.SelectableTextAlign = ImVec2(0.0, 0.0);

        style.Colors[ImGuiCol_Text] = ImVec4(255/255.f, 255/255.f, 255/255.f, 1.0);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(127/255.f, 127/255.f, 127/255.f, 1.0);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(15/255.f, 15/255.f, 15/255.f, 0.9399999976158142);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.0);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(20/255.f, 20/255.f, 20/255.f, 0.9399999976158142);
        style.Colors[ImGuiCol_Border] = ImVec4(109/255.f, 109/255.f, 127/255.f, 0.5);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0/255.f, 0/255.f, 0/255.f, 0.0);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(51/255.f, 53/255.f, 56/255.f, 0.5400000214576721);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(102/255.f, 102/255.f, 102/255.f, 0.4000000059604645);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(45/255.f, 45/255.f, 45/255.f, 0.6700000166893005);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(10/255.f, 10/255.f, 10/255.f, 1.0);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(73/255.f, 73/255.f, 73/255.f, 1.0);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0/255.f, 0/255.f, 0/255.f, 0.5099999904632568);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(35/255.f, 35/255.f, 35/255.f, 1.0);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(5/255.f, 5/255.f, 5/255.f, 0.5299999713897705);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(79/255.f, 79/255.f, 79/255.f, 1.0);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(104/255.f, 104/255.f, 104/255.f, 1.0);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(130/255.f, 130/255.f, 130/255.f, 1.0);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(239/255.f, 239/255.f, 239/255.f, 1.0);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(130/255.f, 130/255.f, 130/255.f, 1.0);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(219/255.f, 219/255.f, 219/255.f, 1.0);
        style.Colors[ImGuiCol_Button] = ImVec4(112/255.f, 112/255.f, 112/255.f, 0.4000000059604645);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(117/255.f, 119/255.f, 122/255.f, 1.0);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(107/255.f, 107/255.f, 107/255.f, 1.0);
        style.Colors[ImGuiCol_Header] = ImVec4(178/255.f, 178/255.f, 178/255.f, 0.3100000023841858);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(178/255.f, 178/255.f, 178/255.f, 0.800000011920929);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(122/255.f, 127/255.f, 132/255.f, 1.0);
        style.Colors[ImGuiCol_Separator] = ImVec4(109/255.f, 109/255.f, 127/255.f, 0.5);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(183/255.f, 183/255.f, 183/255.f, 0.7799999713897705);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(130/255.f, 130/255.f, 130/255.f, 1.0);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(232/255.f, 232/255.f, 232/255.f, 0.25);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(206/255.f, 206/255.f, 206/255.f, 0.6700000166893005);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(117/255.f, 117/255.f, 117/255.f, 0.949999988079071);
        style.Colors[ImGuiCol_Tab] = ImVec4(45/255.f, 89/255.f, 147/255.f, 0.8619999885559082);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(66/255.f, 150/255.f, 249/255.f, 0.800000011920929);
        style.Colors[ImGuiCol_TabActive] = ImVec4(50/255.f, 104/255.f, 173/255.f, 1.0);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(17/255.f, 26/255.f, 37/255.f, 0.9724000096321106);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(34/255.f, 66/255.f, 108/255.f, 1.0);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(155/255.f, 155/255.f, 155/255.f, 1.0);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(255/255.f, 109/255.f, 89/255.f, 1.0);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(186/255.f, 153/255.f, 38/255.f, 1.0);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(255/255.f, 153/255.f, 0/255.f, 1.0);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(48/255.f, 48/255.f, 51/255.f, 1.0);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(79/255.f, 79/255.f, 89/255.f, 1.0);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(58/255.f, 58/255.f, 63/255.f, 1.0);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0/255.f, 0/255.f, 0/255.f, 0.0);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.05999999865889549);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(221/255.f, 221/255.f, 221/255.f, 0.3499999940395355);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(255/255.f, 255/255.f, 0/255.f, 0.8999999761581421);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(153/255.f, 153/255.f, 153/255.f, 1.0);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(255/255.f, 255/255.f, 255/255.f, 0.699999988079071);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(204/255.f, 204/255.f, 204/255.f, 0.2000000029802322);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(204/255.f, 204/255.f, 204/255.f, 0.3499999940395355);
    }
};

#include <imgui.h>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define ZEP_FEATURE_CPP_FILE_SYSTEM
#define ZEP_SINGLE_HEADER_BUILD
#include <zep.h>

#include "State.hpp"
#include "overlay/Editor.hpp"

namespace tat
{

void Editor::create()
{
    window = &State::instance().window;
    zep.zepEditor.SetGlobalMode(Zep::ZepMode_Vim::StaticName());
    zep.zepEditor.GetTheme().SetThemeType(Zep::ThemeType::Dark);
}

void Editor::show()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(window->width, floor(window->height / 3)));

    auto constexpr windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("Zep", nullptr, windowFlags);

    // Fill the window
    auto min = ImGui::GetCursorScreenPos();
    auto max = ImGui::GetContentRegionAvail();

    max.x = min.x + max.x;
    max.y = min.y + max.y;

    zep.zepEditor.RefreshRequired(); // Currently required once per frame
    zep.zepEditor.SetDisplayRegion(Zep::NVec2f(min.x, min.y), Zep::NVec2f(max.x, max.y));
    zep.zepEditor.Display();

    if (ImGui::IsWindowFocused())
    {
        zep.zepEditor.HandleInput();
    }
    ImGui::End();
}

} // namespace tat
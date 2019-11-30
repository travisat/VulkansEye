#include <imgui.h>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define ZEP_FEATURE_CPP_FILE_SYSTEM
#define ZEP_SINGLE_HEADER_BUILD
#include <zep.h>

#include "overlay/Editor.hpp"
#include "State.hpp"

namespace tat
{

void Editor::show()
{
    auto &window = State::instance().window;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(window.width, floor(window.height / 3)));

    auto windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    if (!ImGui::Begin("Zep", nullptr, windowFlags))
    {
        ImGui::End();
        return;
    }

    // Simple menu options for switching mode and splitting
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::BeginMenu("Editor Mode"))
            {
                bool enabledVim = strcmp(zep.GetEditor().GetGlobalMode()->Name(), Zep::ZepMode_Vim::StaticName()) == 0;
                bool enabledNormal = !enabledVim;
                if (ImGui::MenuItem("Vim", "CTRL+2", &enabledVim))
                {
                    zep.GetEditor().SetGlobalMode(Zep::ZepMode_Vim::StaticName());
                }
                else if (ImGui::MenuItem("Standard", "CTRL+1", &enabledNormal))
                {
                    zep.GetEditor().SetGlobalMode(Zep::ZepMode_Standard::StaticName());
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Theme"))
            {
                bool enabledDark = zep.GetEditor().GetTheme().GetThemeType() == Zep::ThemeType::Dark;
                bool enabledLight = !enabledDark;

                if (ImGui::MenuItem("Dark", "", &enabledDark))
                {
                    zep.GetEditor().GetTheme().SetThemeType(Zep::ThemeType::Dark);
                }
                else if (ImGui::MenuItem("Light", "", &enabledLight))
                {
                    zep.GetEditor().GetTheme().SetThemeType(Zep::ThemeType::Light);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            auto pTabWindow = zep.GetEditor().GetActiveTabWindow();
            if (ImGui::MenuItem("Horizontal Split"))
            {
                pTabWindow->AddWindow(&pTabWindow->GetActiveWindow()->GetBuffer(), pTabWindow->GetActiveWindow(),
                                      false);
            }
            else if (ImGui::MenuItem("Vertical Split"))
            {
                pTabWindow->AddWindow(&pTabWindow->GetActiveWindow()->GetBuffer(), pTabWindow->GetActiveWindow(), true);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

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

    // Make the zep window focused on start of the demo - just so the user doesn't start typing without it;
    // not sure why I need to do it twice; something else is stealing the focus the first time round
    static int focus_count = 0;
    if (focus_count++ < 2)
    {
        ImGui::SetWindowFocus();
    }
    ImGui::End();
}

} // namespace tat
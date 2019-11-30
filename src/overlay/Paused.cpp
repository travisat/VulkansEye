#include "overlay/Paused.hpp"
#include "State.hpp"

#include <imgui.h>

namespace tat
{

void Paused::create()
{
    window = &State::instance().window;
}

void Paused::show()
{
    ImGui::SetNextWindowSize(ImVec2(window->width / 3.F, window->height / 3.F));
    ImGui::SetNextWindowPos(ImVec2(window->width / 3.F, window->height / 3.F));
    auto windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("Paused", nullptr, windowFlags);
    ImGui::Text("Are you sure you want to close?");
    
    if (ImGui::Button("Yes", ImVec2(window->width/ 4.F, window->height / 6.F)))
    {
        close();
    }
    ImGui::End();
}

void Paused::close()
{
    window->setClose(1);
}

} // namespace tat
#include "overlay/Info.hpp"

#include "Timer.hpp"
#include "State.hpp"

namespace tat
{

void Info::create()
{
    player = &State::instance().player;
    camera = &State::instance().camera;
}

void Info::show(float deltaTime)
{
    float frameTime = Timer::time();
    data.lastFrameTime = frameTime;
    if (((frameTime - data.lastUpdateTime) > data.updateFreqTime) || (data.lastUpdateTime == 0.F))
    {
        data.lastUpdateTime = frameTime;
        data.fps = 1.F / deltaTime;
        data.position = player->position();
        data.rotation = camera->rotation();
    }
    ImGui::SetNextWindowSize(ImVec2(320, 120));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    auto windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("Info", nullptr, windowFlags);
    ImGui::InputFloat("Fps", &data.fps, 0.F, 0.F, "%.1f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Position", &data.position.x, "%.1f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat3("Rotation", &data.rotation.x, "%.1f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputText("Test", data.buffer.data(), data.buffer.size());
    ImGui::End();
}

}  // namespace tat
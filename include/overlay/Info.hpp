#pragma once

#include <array>
#include <cstdint>
#include <string_view>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "Player.hpp"

namespace tat
{

constexpr std::array<std::string_view, 4> mode = {"Normal", "Visual", "Insert", "Paused"};

class Info
{
  public:
    void create();
    void show(float deltaTime);

  private:
    Player *player = nullptr;
    Camera *camera = nullptr;

    struct
    {
        glm::vec3 position = glm::vec3(0.F);
        glm::vec3 rotation = glm::vec3(0.F);

        std::array<char, 10> buffer;

        float velocity = 0.F;
        float fps = 0.F;
        int32_t modeNum = 0;
        float lastFrameTime = 0.F;
        float lastUpdateTime = 0.F;
        float updateFreqTime = 0.1F; // time between updates
    } data;
};

} // namespace tat
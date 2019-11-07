#pragma once

#include "Config.hpp"
#include "Vulkan.hpp"
#include "Model.hpp"
#include <memory>

namespace tat
{
class Light
{
  public:
    std::shared_ptr<Vulkan> vulkan;
    LightConfig config;

    std::string name;
    glm::vec4 position{};
    glm::vec4 color{};
    float lumens = 0.0F;
    void create();

  private:
    static auto kelvinToRGB(float kelvin) -> glm::vec3;
};

} // namespace tat
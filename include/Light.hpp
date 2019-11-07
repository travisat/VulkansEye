#pragma once

#include "Config.hpp"
#include "Vulkan.hpp"
#include "Model.hpp"

namespace tat
{
class Light
{
  public:
    Vulkan *vulkan;
    LightConfig *config;

    std::string name;
    UniformLight light{};
    void create();

  private:
    static auto kelvinToRGB(float kelvin) -> glm::vec3;
};

} // namespace tat
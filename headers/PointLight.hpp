#pragma once

#include "Config.h"

#include "Vulkan.hpp"
#include "Model.hpp"

namespace tat
{
class PointLight
{
  public:
    Vulkan *vulkan;
    PointLightConfig *config;

    uint32_t id;
    std::string name;
    uPointLight light{};
    void create();

  private:
    static auto kelvinToRGB(float kelvin) -> glm::vec3;
};

} // namespace tat
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
    Model model;
    void create();

  private:
    glm::vec3 kelvinToRGB(float kelvin);
};

} // namespace tat
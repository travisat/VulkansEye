#pragma once

#include "Config.h"

#include "Vulkan.hpp"
#include "Model.hpp"

namespace tat
{
class Light
{
  public:
    Vulkan *vulkan;
    LightConfig *config;

    uint32_t id;
    std::string name;
    uLight light{};
    void create();

  private:
    static auto kelvinToRGB(float kelvin) -> glm::vec3;
};

} // namespace tat
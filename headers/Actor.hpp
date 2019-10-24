#pragma once

#include "Config.h"

#include "Buffer.hpp"
#include "Model.hpp"
#include "Vulkan.hpp"

namespace tat
{

class Actor
{
  public:
    // config values
    tat::Vulkan *vulkan = nullptr;
    ActorConfig *config;
    Image *shadow;
    VkSampler shadowSampler;

    // generated values
    std::string name = "Unknown Actor";

    Model model;

    void create();

  private:
};

} // namespace tat
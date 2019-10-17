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

    // generated values
    std::string name = "Unknown Actor";

    Model model;

    void create();

  private:
};

} // namespace tat
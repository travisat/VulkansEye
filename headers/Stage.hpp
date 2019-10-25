#pragma once

#include "Config.h"
#include "Vertex.h"

#include "Backdrop.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Model.hpp"
#include "Player.hpp"

namespace tat
{

class Stage
{
  public:
    Vulkan *vulkan = nullptr;
    StageConfig *config;
    Player *player;

    Backdrop backdrop;
    std::vector<Model> models;

    Image *shadow;
    VkSampler shadowSampler;

    void create();
    void recreate()
    {
        backdrop.recreate();
    };
    void cleanup()
    {
        backdrop.cleanup();
    };
   
    void createColorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void createShadowSets(VkDescriptorPool pool, VkDescriptorSetLayout layout);

  private:
};

} // namespace tat
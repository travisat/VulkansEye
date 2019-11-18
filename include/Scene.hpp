#pragma once

#include <memory>

#include "Backdrop.hpp"
#include "Image.hpp"
#include "Model.hpp"
#include "engine/Pipeline.hpp"


namespace tat
{

class Scene
{
  public:
    std::string name = "Unknown";

    Image shadow {};
    Image brdf {};
    Backdrop *backdrop = nullptr;

    float shadowSize = 1024.F;

    void destroy();
    void create();
    void recreate();
    void drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage, float deltaTime);

  private:
    Pipeline colorPipeline;
    Pipeline shadowPipeline;
    
    vk::DescriptorPool colorPool;
    vk::DescriptorSetLayout colorLayout;
    vk::DescriptorPool shadowPool;
    vk::DescriptorSetLayout shadowLayout;

    UniformVert vertBuffer{};
    UniformFrag fragBuffer{};
    UniformShad shadBuffer{};

    std::vector<Model*> models{};

    void createBrdf();
    void createShadow();

    void loadModels();
    void loadBackdrop();

    void createColorPool();
    void createColorLayouts();
    void createColorPipeline();
    void createColorSets();

    void createShadowPool();
    void createShadowLayouts();
    void createShadowPipeline();
    void createShadowSets();
};

} // namespace tat
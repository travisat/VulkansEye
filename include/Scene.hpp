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
    Scene() = default;
    ~Scene();

    std::string name = "Unknown";

    std::shared_ptr<Image> shadow;
    std::shared_ptr<Image> brdf;
    std::shared_ptr<Backdrop> backdrop;

    float shadowSize = 1024.F;

    void load();
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

    std::vector<std::shared_ptr<Model>> models{};

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
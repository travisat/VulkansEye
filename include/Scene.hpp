#pragma once

#include <memory>

#include "Image.hpp"
#include "Pipeline.hpp"
#include "Model.hpp"
#include "Backdrop.hpp"

namespace tat
{

class Scene
{
  public:
    std::string name = "Unknown";

    std::shared_ptr<Image> shadow;
    std::shared_ptr<Image> brdf;
    std::shared_ptr<Backdrop> backdrop;

    float shadowSize = 1024.F;

    Scene() = default;
    ~Scene();

    void load();
    void recreate();
    void drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage, float deltaTime);

  private:
    vk::UniqueDescriptorPool colorPool;
    vk::UniqueDescriptorSetLayout colorLayout;
    vk::UniqueDescriptorPool shadowPool;
    vk::UniqueDescriptorSetLayout shadowLayout;

    UniformVert vertBuffer{};
    UniformFrag fragBuffer{};
    UniformShad shadBuffer{};

    Pipeline colorPipeline;
    std::vector<std::shared_ptr<Model>> models {};

    Pipeline shadowPipeline;

    void createBrdf();
    void createShadow();

    void loadModels();
    void loadBackdrop();

    void createColorPool();
    void createColorLayouts();
    void createColorPipeline();
    void createColorSets();
    void recreateColorSets();

    void createShadowPool();
    void createShadowLayouts();
    void createShadowPipeline();
    void createShadowSets();
};

} // namespace tat
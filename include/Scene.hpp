#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

#include "Backdrops.hpp"
#include "Config.hpp"
#include "Materials.hpp"
#include "Meshes.hpp"
#include "Model.hpp"
#include "Pipeline.hpp"
#include "Player.hpp"
#include "Camera.hpp"
#include "Vulkan.hpp"

namespace tat
{

class Scene
{
  public:
    std::string name = "Unknown";

    Image shadow;
    Image brdf;

    Scene(const std::shared_ptr<Vulkan> &vulkan, const std::shared_ptr<Camera> &camera, const std::shared_ptr<Player> &player,
          const std::shared_ptr<Materials> &materials, const std::shared_ptr<Meshes> &meshes,
          const std::shared_ptr<Backdrops> &backdrops, const std::string &configPath);
    ~Scene();

    void cleanup();
    void recreate();
    void drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage);
    void update(uint32_t currentImage, float deltaTime);

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
    std::shared_ptr<Vulkan> vulkan;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Player> player;
    std::shared_ptr<Materials> materials;
    std::shared_ptr<Meshes> meshes;
    std::shared_ptr<Backdrops> backdrops;

    vk::DescriptorPool colorPool;
    vk::DescriptorSetLayout colorLayout;
    vk::DescriptorPool shadowPool;
    vk::DescriptorSetLayout shadowLayout;

    Backdrop *backdrop = nullptr;

    UniformVert vertBuffer{};
    UniformFrag fragBuffer{};
    UniformShad shadBuffer{};

    Pipeline colorPipeline;
    std::vector<Model> models;

    Pipeline shadowPipeline;

    void createBrdf();
    void createShadow();
    void createModels(const std::vector<ModelConfig> &configs);

    void loadBackdrop(const std::string &name);

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
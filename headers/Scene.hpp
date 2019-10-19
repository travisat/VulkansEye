#pragma once

#include <chrono>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <utility>


#include "Actor.hpp"
#include "Backdrop.hpp"
#include "Config.h"
#include "Pipeline.hpp"
#include "Player.hpp"
#include "PointLight.hpp"
#include "Stage.hpp"
#include "Vulkan.hpp"


namespace tat {

class Scene {
public:
  // config values
  Vulkan *vulkan = nullptr;
  SceneConfig *config = nullptr;
  Player *player = nullptr;

  // generated values
  std::string name = "Unknown";
  Stage stage;
  Pipeline pipeline;
  std::vector<Actor> actors;
  std::vector<PointLight> pointLights;

  ~Scene();

  void create();
  void cleanup();
  void recreate();
  void draw(VkCommandBuffer commandBuffer, uint32_t currentImage);
  void update(uint32_t currentImage);

  uint32_t numTessBuffers() {
    return static_cast<uint32_t>((actors.size() + stage.models.size()) * 2);
  };
  uint32_t numUniformLights() {
    return static_cast<uint32_t>(actors.size() + stage.models.size());
  };
  uint32_t numImageSamplers() {
    return static_cast<uint32_t>((actors.size() + stage.models.size()) * 6);
  };

private:
  UniformLight uLight = {};
  VkDescriptorPool descriptorPool;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSetLayout offscreenLayout;

  void createLights();
  void createActors();
  void createBackdrop();
  void createStage();

  void createDescriptorPool();
  void createDescriptorSetLayouts();
  void createPipelines();
  void createUniformBuffers();
  void createDescriptorSets();
};

} // namespace tat
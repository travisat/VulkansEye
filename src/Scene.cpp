#include "Scene.hpp"
#include "helpers.h"

namespace tat {

Scene::~Scene() {
  vkDestroyDescriptorSetLayout(vulkan->device, descriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Scene::create() {
  createStage();
  createLights();
  createActors();

  createDescriptorPool();
  createDescriptorSetLayouts();
  createPipeline();
  createUniformBuffers();
  createDescriptorSets();
}

void Scene::createStage() {
  stage.vulkan = vulkan;
  stage.config = &config->stageConfig;
  stage.player = player;

  stage.create();
}

void Scene::createLights() {
  pointLights.resize(numLights);
  for (auto &lightConfig : config->pointLights) {
    pointLights[lightConfig.index].config = &lightConfig;
    pointLights[lightConfig.index].load();
  }
}

void Scene::createActors() {
  actors.resize(config->actors.size());
  for (auto &actorConfig : config->actors) {
    actors[actorConfig.index].config = &actorConfig;
    actors[actorConfig.index].vulkan = vulkan;
    actors[actorConfig.index].create();
  }
}

void Scene::cleanup() {
  stage.cleanup();
  pipeline.cleanup();
  vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Scene::recreate() {
  player->updateAspectRatio((float)vulkan->width, (float)vulkan->height);
  stage.recreate();
  createPipeline();
  createUniformBuffers();
  createDescriptorPool();
  createDescriptorSets();
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentImage) {
  VkViewport viewport{};
  viewport.width = (float)vulkan->width;
  viewport.height = (float)vulkan->height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.extent = vulkan->swapChainExtent;
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  vkCmdSetLineWidth(commandBuffer, 1.0f);

  stage.backdrop.draw(commandBuffer, currentImage);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline.pipeline);

  VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &stage.vertexBuffer.buffer,
                         offsets);
  vkCmdBindIndexBuffer(commandBuffer, stage.indexBuffer.buffer, 0,
                       VK_INDEX_TYPE_UINT32);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline.pipelineLayout, 0, 1,
                          &stage.descriptorSets[currentImage], 0, nullptr);
  vkCmdDrawIndexed(commandBuffer, stage.model.indexSize, 1, 0, 0, 0);

  for (auto &actor : actors) {
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &actor.vertexBuffer.buffer,
                           offsets);
    vkCmdBindIndexBuffer(commandBuffer, actor.indexBuffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline.pipelineLayout, 0, 1,
                            &actor.descriptorSets[currentImage], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, actor.model.indexSize, 1, 0, 0, 0);
  }
}

void Scene::createUniformBuffers() {
  stage.createUniformBuffers();

  for (auto &actor : actors) {
    actor.createUniformBuffers();
  }
}

void Scene::update(uint32_t currentImage) {
  stage.backdrop.update(currentImage);
  for (int32_t i = 0; i < numLights; ++i) {
    uLight.light[i].position = pointLights[i].light.position;
    uLight.light[i].temperature = pointLights[i].light.temperature;
    uLight.light[i].lumens = pointLights[i].light.lumens;
  }
  uLight.light[0].position.x = -player->position.x;
  uLight.light[0].position.z = -player->position.z;

  stage.model.uBuffer.projection = player->perspective;
  stage.model.uBuffer.view = player->view;
  stage.model.uBuffer.model = glm::mat4(1.0f);

  stage.uniformBuffers[currentImage].update(&stage.model.uBuffer,
                                            sizeof(UniformBuffer));
  stage.uniformLights[currentImage].update(&uLight, sizeof(uLight));

  for (auto &actor : actors) {
    actor.model.uBuffer.projection = player->perspective;
    actor.model.uBuffer.view = player->view;
    actor.model.uBuffer.model = glm::translate(glm::mat4(1.0f), actor.position);
    actor.model.uBuffer.model =
        glm::rotate(actor.model.uBuffer.model, glm::radians(actor.rotation.x),
                    glm::vec3(1.0f, 0.0f, 0.0f));
    actor.model.uBuffer.model =
        glm::rotate(actor.model.uBuffer.model, glm::radians(actor.rotation.y),
                    glm::vec3(0.0f, 1.0f, 0.0f));
    actor.model.uBuffer.model =
        glm::rotate(actor.model.uBuffer.model, glm::radians(actor.rotation.z),
                    glm::vec3(0.0f, 0.0f, 1.0f));
    actor.model.uBuffer.model =
        glm::scale(actor.model.uBuffer.model, actor.scale);

    actor.uniformLights[currentImage].update(&uLight, sizeof(uLight));
    actor.uniformBuffers[currentImage].update(&actor.model.uBuffer,
                                              sizeof(UniformBuffer));
  }
}

void Scene::createDescriptorPool() {
  uint32_t numSwapChainImages =
      static_cast<uint32_t>(vulkan->swapChainImages.size());

  std::array<VkDescriptorPoolSize, 2> poolSizes = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount =
      (numTessBuffers() + numUniformLights() * numLights) * numSwapChainImages;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = numImageSamplers() * numSwapChainImages;

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets =
      (numTessBuffers() + numUniformLights() + numImageSamplers() * numLights) *
      numSwapChainImages;

  CheckResult(vkCreateDescriptorPool(vulkan->device, &poolInfo, nullptr,
                                     &descriptorPool));
}

void Scene::createDescriptorSetLayouts() {
  VkDescriptorSetLayoutBinding uBufferLayoutBinding = {};
  uBufferLayoutBinding.binding = 0;
  uBufferLayoutBinding.descriptorCount = 1;
  uBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uBufferLayoutBinding.pImmutableSamplers = nullptr;
  uBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding uLightLayoutBinding = {};
  uLightLayoutBinding.binding = 1;
  uLightLayoutBinding.descriptorCount = 1;
  uLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uLightLayoutBinding.pImmutableSamplers = nullptr;
  uLightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding diffuseLayoutBinding = {};
  diffuseLayoutBinding.binding = 2;
  diffuseLayoutBinding.descriptorCount = 1;
  diffuseLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  diffuseLayoutBinding.pImmutableSamplers = nullptr;
  diffuseLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding normalLayoutBinding = {};
  normalLayoutBinding.binding = 3;
  normalLayoutBinding.descriptorCount = 1;
  normalLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  normalLayoutBinding.pImmutableSamplers = nullptr;
  normalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding roughnessLayoutBinding = {};
  roughnessLayoutBinding.binding = 4;
  roughnessLayoutBinding.descriptorCount = 1;
  roughnessLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  roughnessLayoutBinding.pImmutableSamplers = nullptr;
  roughnessLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding metallicLayoutBinding = {};
  metallicLayoutBinding.binding = 5;
  metallicLayoutBinding.descriptorCount = 1;
  metallicLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  metallicLayoutBinding.pImmutableSamplers = nullptr;
  metallicLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding aoLayoutBinding = {};
  aoLayoutBinding.binding = 6;
  aoLayoutBinding.descriptorCount = 1;
  aoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  aoLayoutBinding.pImmutableSamplers = nullptr;
  aoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding dispLayoutBinding = {};
  dispLayoutBinding.binding = 7;
  dispLayoutBinding.descriptorCount = 1;
  dispLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  dispLayoutBinding.pImmutableSamplers = nullptr;
  dispLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 8> bindings = {
      uBufferLayoutBinding, uLightLayoutBinding,    diffuseLayoutBinding,
      normalLayoutBinding,  roughnessLayoutBinding, metallicLayoutBinding,
      aoLayoutBinding,      dispLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  CheckResult(vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, nullptr,
                                          &descriptorSetLayout));
}

void Scene::createDescriptorSets() {
  stage.createDescriptorSets(descriptorPool, descriptorSetLayout);

  for (auto &actor : actors) {
    actor.createDescriptorSets(descriptorPool, descriptorSetLayout);
  }
}

void Scene::createPipeline() {
  pipeline.vulkan = vulkan;
  pipeline.descriptorSetLayout = descriptorSetLayout;
  pipeline.vertShaderPath = "resources/shaders/scene.vert.spv";
  pipeline.fragShaderPath = "resources/shaders/scene.frag.spv";
  pipeline.tescShaderPath = "resources/shaders/displacement.tesc.spv";
  pipeline.teseShaderPath = "resources/shaders/displacement.tese.spv";
  pipeline.createScenePipeline();
}

} // namespace tat
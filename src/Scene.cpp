#include "Scene.hpp"
#include "helpers.h"

namespace tat
{

Scene::~Scene()
{
    vkDestroyDescriptorSetLayout(vulkan->device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Scene::create()
{
    createStage();
    createLights();
    createActors();

    createDescriptorPool();
    createDescriptorSetLayouts();
    createPipeline();
    createUniformBuffers();
    createDescriptorSets();
}

void Scene::createStage()
{
    stage.vulkan = vulkan;
    stage.config = &config->stageConfig;
    stage.player = player;
    stage.create();
}

void Scene::createLights()
{
    pointLights.resize(numLights);
    for (auto &lightConfig : config->pointLights)
    {
        pointLights[lightConfig.index].config = &lightConfig;
        pointLights[lightConfig.index].load();
    }
}

void Scene::createActors()
{
    actors.resize(config->actors.size());
    for (auto &actorConfig : config->actors)
    {
        actors[actorConfig.index].config = &actorConfig;
        actors[actorConfig.index].vulkan = vulkan;
        actors[actorConfig.index].create();
    }
}

void Scene::cleanup()
{
    stage.cleanup();
    pipeline.cleanup();
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Scene::recreate()
{
    player->updateAspectRatio((float)vulkan->width, (float)vulkan->height);
    stage.recreate();
    createPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    stage.backdrop.draw(commandBuffer, currentImage);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &stage.vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, stage.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &stage.descriptorSets[currentImage], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, stage.model.indexSize, 1, 0, 0, 0);

    for (auto &actor : actors)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &actor.vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, actor.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &actor.descriptorSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, actor.model.indexSize, 1, 0, 0, 0);
    }
}

void Scene::createUniformBuffers()
{
    stage.createUniformBuffers();

    for (auto &actor : actors)
    {
        actor.createUniformBuffers();
    }
}

void Scene::updateUniformBuffer(uint32_t currentImage)
{

    uBuffer.model = glm::mat4(1.0f);
    uBuffer.projection = player->perspective;
    uBuffer.view = player->view;

    uTessEval.projection = player->perspective;

    for (int32_t i = 0; i < numLights; ++i)
    {
        uLight.light[i].position = pointLights[i].light.position;
        uLight.light[i].temperature = pointLights[i].light.temperature;
        uLight.light[i].lumens = pointLights[i].light.lumens;
    }

    uLight.light[0].position = player->position * -1.0f;

    uTessEval.model = uBuffer.model;
    stage.backdrop.updateUniformBuffer(currentImage);
    stage.uniformBuffers[currentImage].update(&uBuffer, sizeof(UniformBuffer));
    stage.tescBuffers[currentImage].update(&uTessControl, sizeof(TessControl));
    stage.teseBuffers[currentImage].update(&uTessEval, sizeof(TessEval));
    stage.uniformLights[currentImage].update(&uLight, sizeof(UniformLight));

    for (auto &actor : actors)
    {
        uBuffer.model = glm::translate(glm::mat4(1.0f), actor.position);
        uBuffer.model = glm::rotate(uBuffer.model, glm::radians(actor.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        uBuffer.model = glm::rotate(uBuffer.model, glm::radians(actor.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        uBuffer.model = glm::rotate(uBuffer.model, glm::radians(actor.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        uTessEval.model = uBuffer.model;

        actor.uniformBuffers[currentImage].update(&uBuffer, sizeof(UniformBuffer));
        actor.tescBuffers[currentImage].update(&uTessControl, sizeof(TessControl));
        actor.teseBuffers[currentImage].update(&uTessEval, sizeof(TessEval));
        actor.uniformLights[currentImage].update(&uLight, sizeof(UniformLight));
    }
}

void Scene::createDescriptorPool()
{
    uint32_t numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (numUniformBuffers() + +numTessBuffers() + numUniformLights() * numLights) * numSwapChainImages;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = numImageSamplers() * numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = (numUniformBuffers() + numTessBuffers() + numUniformLights() + numImageSamplers() * numLights) * numSwapChainImages;

    CheckResult(vkCreateDescriptorPool(vulkan->device, &poolInfo, nullptr, &descriptorPool));
}

void Scene::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uBufferLayoutBinding = {};
    uBufferLayoutBinding.binding = 0;
    uBufferLayoutBinding.descriptorCount = 1;
    uBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uBufferLayoutBinding.pImmutableSamplers = nullptr;
    uBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding uTessControlLayoutBinding = {};
    uTessControlLayoutBinding.binding = 1;
    uTessControlLayoutBinding.descriptorCount = 1;
    uTessControlLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uTessControlLayoutBinding.pImmutableSamplers = nullptr;
    uTessControlLayoutBinding.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    VkDescriptorSetLayoutBinding uTessEvalLayoutBinding = {};
    uTessEvalLayoutBinding.binding = 2;
    uTessEvalLayoutBinding.descriptorCount = 1;
    uTessEvalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uTessEvalLayoutBinding.pImmutableSamplers = nullptr;
    uTessEvalLayoutBinding.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    VkDescriptorSetLayoutBinding dispLayoutBinding = {};
    dispLayoutBinding.binding = 3;
    dispLayoutBinding.descriptorCount = 1;
    dispLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dispLayoutBinding.pImmutableSamplers = nullptr;
    dispLayoutBinding.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    VkDescriptorSetLayoutBinding uLightLayoutBinding = {};
    uLightLayoutBinding.binding = 4;
    uLightLayoutBinding.descriptorCount = 1;
    uLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uLightLayoutBinding.pImmutableSamplers = nullptr;
    uLightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding diffuseLayoutBinding = {};
    diffuseLayoutBinding.binding = 5;
    diffuseLayoutBinding.descriptorCount = 1;
    diffuseLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    diffuseLayoutBinding.pImmutableSamplers = nullptr;
    diffuseLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalLayoutBinding = {};
    normalLayoutBinding.binding = 6;
    normalLayoutBinding.descriptorCount = 1;
    normalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalLayoutBinding.pImmutableSamplers = nullptr;
    normalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding roughnessLayoutBinding = {};
    roughnessLayoutBinding.binding = 7;
    roughnessLayoutBinding.descriptorCount = 1;
    roughnessLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    roughnessLayoutBinding.pImmutableSamplers = nullptr;
    roughnessLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding metallicLayoutBinding = {};
    metallicLayoutBinding.binding = 8;
    metallicLayoutBinding.descriptorCount = 1;
    metallicLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    metallicLayoutBinding.pImmutableSamplers = nullptr;
    metallicLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding aoLayoutBinding = {};
    aoLayoutBinding.binding = 9;
    aoLayoutBinding.descriptorCount = 1;
    aoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    aoLayoutBinding.pImmutableSamplers = nullptr;
    aoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 10> bindings =
        {uBufferLayoutBinding,
         uTessControlLayoutBinding,
         uTessEvalLayoutBinding,
         dispLayoutBinding,
         uLightLayoutBinding,
         diffuseLayoutBinding,
         normalLayoutBinding,
         roughnessLayoutBinding,
         metallicLayoutBinding,
         aoLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    CheckResult(vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, nullptr, &descriptorSetLayout));
}

void Scene::createDescriptorSets()
{
    stage.createDescriptorSets(descriptorPool, descriptorSetLayout);

    for (auto &actor : actors)
    {
        actor.createDescriptorSets(descriptorPool, descriptorSetLayout);
    }
}

void Scene::createPipeline()
{
    pipeline.vulkan = vulkan;
    pipeline.descriptorSetLayout = descriptorSetLayout;
    pipeline.vertShaderPath = "resources/shaders/scene.vert.spv";
    pipeline.fragShaderPath = "resources/shaders/scene.frag.spv";
    pipeline.tescShaderPath = "resources/shaders/displacement.tesc.spv";
    pipeline.teseShaderPath = "resources/shaders/displacement.tese.spv";
    pipeline.createScenePipeline();
}

} //namespace tat
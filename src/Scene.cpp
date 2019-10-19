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
    createPipelines();
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
    offscreenPipeline.cleanup();
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Scene::recreate()
{
    player->updateAspectRatio((float)vulkan->width, (float)vulkan->height);
    stage.recreate();
    createPipelines();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void Scene::draw(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    stage.backdrop.draw(commandBuffer, currentImage);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

    VkDeviceSize offsets[1] = {0};
    for (auto &model : stage.models)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model.vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1,
                                &model.descriptorSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, model.indexSize, 1, 0, 0, 0);
    }
    for (auto &actor : actors)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &actor.model.vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, actor.model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1,
                                &actor.model.descriptorSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, actor.model.indexSize, 1, 0, 0, 0);
    }
}

void Scene::createUniformBuffers()
{
    stage.createUniformBuffers();
    for (auto &actor : actors)
    {
        actor.model.createUniformBuffers();
    }
}

void Scene::update(uint32_t currentImage)
{
    stage.backdrop.update(currentImage);
    for (int32_t i = 0; i < numLights; ++i)
    {
        uLight.light[i].position = pointLights[i].light.position;
        uLight.light[i].temperature = pointLights[i].light.temperature;
        uLight.light[i].lumens = pointLights[i].light.lumens;
    }
    uLight.light[0].position.x = -player->position.x;
    uLight.light[0].position.z = -player->position.z;

    for (auto &model : stage.models)
    {
        model.uTessEval.projection = player->perspective;
        model.uTessEval.view = player->view;
        model.uTessEval.model = glm::translate(glm::mat4(1.0f), model.position);
        model.uTessEval.model =
            glm::rotate(model.uTessEval.model, glm::radians(model.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model.uTessEval.model =
            glm::rotate(model.uTessEval.model, glm::radians(model.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model.uTessEval.model =
            glm::rotate(model.uTessEval.model, glm::radians(model.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model.uTessEval.model = glm::scale(model.uTessEval.model, model.scale);

        model.tescBuffers[currentImage].update(&model.uTessControl, sizeof(model.uTessControl));
        model.teseBuffers[currentImage].update(&model.uTessEval, sizeof(model.uTessEval));
        model.uniformLights[currentImage].update(&uLight, sizeof(uLight));
    }

    for (auto &actor : actors)
    {
        actor.model.uTessEval.projection = player->perspective;
        actor.model.uTessEval.view = player->view;
        actor.model.uTessEval.model = glm::translate(glm::mat4(1.0f), actor.model.position);
        actor.model.uTessEval.model =
            glm::rotate(actor.model.uTessEval.model, glm::radians(actor.model.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        actor.model.uTessEval.model =
            glm::rotate(actor.model.uTessEval.model, glm::radians(actor.model.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        actor.model.uTessEval.model =
            glm::rotate(actor.model.uTessEval.model, glm::radians(actor.model.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        actor.model.uTessEval.model = glm::scale(actor.model.uTessEval.model, actor.model.scale);

        actor.model.tescBuffers[currentImage].update(&actor.model.uTessControl, sizeof(actor.model.uTessControl));
        actor.model.teseBuffers[currentImage].update(&actor.model.uTessEval, sizeof(actor.model.uTessEval));
        actor.model.uniformLights[currentImage].update(&uLight, sizeof(uLight));
    }
}

void Scene::createDescriptorPool()
{
    uint32_t numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (numTessBuffers() + numUniformLights() * numLights) * numSwapChainImages;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = numImageSamplers() * numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = (numTessBuffers() + numUniformLights() + numImageSamplers() * numLights) * numSwapChainImages;

    CheckResult(vkCreateDescriptorPool(vulkan->device, &poolInfo, nullptr, &descriptorPool));
}

void Scene::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uTessControlLayoutBinding = {};
    uTessControlLayoutBinding.binding = 0;
    uTessControlLayoutBinding.descriptorCount = 1;
    uTessControlLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uTessControlLayoutBinding.pImmutableSamplers = nullptr;
    uTessControlLayoutBinding.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

    VkDescriptorSetLayoutBinding uTessEvalLayoutBinding = {};
    uTessEvalLayoutBinding.binding = 1;
    uTessEvalLayoutBinding.descriptorCount = 1;
    uTessEvalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uTessEvalLayoutBinding.pImmutableSamplers = nullptr;
    uTessEvalLayoutBinding.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    VkDescriptorSetLayoutBinding dispLayoutBinding = {};
    dispLayoutBinding.binding = 2;
    dispLayoutBinding.descriptorCount = 1;
    dispLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dispLayoutBinding.pImmutableSamplers = nullptr;
    dispLayoutBinding.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    VkDescriptorSetLayoutBinding uLightLayoutBinding = {};
    uLightLayoutBinding.binding = 3;
    uLightLayoutBinding.descriptorCount = 1;
    uLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uLightLayoutBinding.pImmutableSamplers = nullptr;
    uLightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding diffuseLayoutBinding = {};
    diffuseLayoutBinding.binding = 4;
    diffuseLayoutBinding.descriptorCount = 1;
    diffuseLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    diffuseLayoutBinding.pImmutableSamplers = nullptr;
    diffuseLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalLayoutBinding = {};
    normalLayoutBinding.binding = 5;
    normalLayoutBinding.descriptorCount = 1;
    normalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalLayoutBinding.pImmutableSamplers = nullptr;
    normalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding roughnessLayoutBinding = {};
    roughnessLayoutBinding.binding = 6;
    roughnessLayoutBinding.descriptorCount = 1;
    roughnessLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    roughnessLayoutBinding.pImmutableSamplers = nullptr;
    roughnessLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding metallicLayoutBinding = {};
    metallicLayoutBinding.binding = 7;
    metallicLayoutBinding.descriptorCount = 1;
    metallicLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    metallicLayoutBinding.pImmutableSamplers = nullptr;
    metallicLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding aoLayoutBinding = {};
    aoLayoutBinding.binding = 8;
    aoLayoutBinding.descriptorCount = 1;
    aoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    aoLayoutBinding.pImmutableSamplers = nullptr;
    aoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 9> bindings = {
        uTessControlLayoutBinding, uTessEvalLayoutBinding, dispLayoutBinding,
        uLightLayoutBinding,       diffuseLayoutBinding,   normalLayoutBinding,
        roughnessLayoutBinding,    metallicLayoutBinding,  aoLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    CheckResult(vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, nullptr, &descriptorSetLayout));

    //offscreen layout

    VkDescriptorSetLayoutBinding offscreenVertexLayoutBinding = {};
    offscreenVertexLayoutBinding.binding = 0;
    offscreenVertexLayoutBinding.descriptorCount = 1;
    offscreenVertexLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    offscreenVertexLayoutBinding.pImmutableSamplers = nullptr;
    offscreenVertexLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding offscreenFragmentLayoutBinding = {};
    offscreenFragmentLayoutBinding.binding = 1;
    offscreenFragmentLayoutBinding.descriptorCount = 1;
    offscreenFragmentLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    offscreenFragmentLayoutBinding.pImmutableSamplers = nullptr;
    offscreenFragmentLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> offscreenBindings = {offscreenVertexLayoutBinding,
                                                                     offscreenFragmentLayoutBinding};

    VkDescriptorSetLayoutCreateInfo offscreenLayoutInfo = {};
    offscreenLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    offscreenLayoutInfo.bindingCount = static_cast<uint32_t>(offscreenBindings.size());
    offscreenLayoutInfo.pBindings = offscreenBindings.data();

    CheckResult(vkCreateDescriptorSetLayout(vulkan->device, &offscreenLayoutInfo, nullptr, &offscreenLayout));
}

void Scene::createDescriptorSets()
{
    stage.createDescriptorSets(descriptorPool, descriptorSetLayout);

    for (auto &actor : actors)
    {
        actor.model.createDescriptorSets(descriptorPool, descriptorSetLayout);
    }
}

void Scene::createPipelines()
{
    pipeline.vulkan = vulkan;
    pipeline.descriptorSetLayout = descriptorSetLayout;
    pipeline.loadDefaults();

    auto vertShaderCode = readFile("resources/shaders/scene.vert.spv");
    auto fragShaderCode = readFile("resources/shaders/scene.frag.spv");
    auto tessControlShaderCode = readFile("resources/shaders/displacement.tesc.spv");
    auto tessEvalShaderCode = readFile("resources/shaders/displacement.tese.spv");

    pipeline.vertShaderStageInfo.module = vulkan->createShaderModule(vertShaderCode);
    pipeline.fragShaderStageInfo.module = vulkan->createShaderModule(fragShaderCode);
    pipeline.tescShaderStageInfo.module = vulkan->createShaderModule(tessControlShaderCode);
    pipeline.teseShaderStageInfo.module = vulkan->createShaderModule(tessEvalShaderCode);

    pipeline.shaderStages = {pipeline.vertShaderStageInfo, pipeline.fragShaderStageInfo, pipeline.tescShaderStageInfo,
                             pipeline.teseShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    pipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    pipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    pipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    pipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    pipeline.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    pipeline.pipelineInfo.pTessellationState = &pipeline.tessellationState;
    pipeline.create();

    //offscreenPipeline

    offscreenPipeline.vulkan = vulkan;
    offscreenPipeline.descriptorSetLayout = offscreenLayout;
    offscreenPipeline.loadDefaults();

    auto offscreenVertShaderCode = readFile("resources/shaders/offscreen.vert");
    auto offscreenFragShaderCode = readFile("resources/shaders/offscreen.frag");
    offscreenPipeline.vertShaderStageInfo.module = vulkan->createShaderModule(offscreenVertShaderCode);
    offscreenPipeline.fragShaderStageInfo.module = vulkan->createShaderModule(offscreenFragShaderCode);

    offscreenPipeline.shaderStages = {offscreenPipeline.vertShaderStageInfo, offscreenPipeline.fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    offscreenPipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    offscreenPipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    offscreenPipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    offscreenPipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

     // Push constants for cubeMap
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.size = sizeof(glm::mat4);
    pushConstantRange.offset = 0;

    offscreenPipeline.pipelineLayoutInfo.pushConstantRangeCount = 1;
    offscreenPipeline.pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    offscreenPipeline.create();
}

} // namespace tat
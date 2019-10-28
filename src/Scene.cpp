#include "Scene.hpp"
#include "Vulkan.hpp"
#include "helpers.h"
#include "vulkan/vulkan_core.h"

namespace tat
{

Scene::~Scene()
{
    vkDestroyDescriptorSetLayout(vulkan->device, shadowLayout, nullptr);
    vkDestroyDescriptorSetLayout(vulkan->device, colorLayout, nullptr);
    vkDestroyDescriptorPool(vulkan->device, colorPool, nullptr);
    vkDestroyDescriptorPool(vulkan->device, shadowPool, nullptr);
}

void Scene::create()
{
    createShadow();
    createLights();

    createMaterials();
    createStage();
    createActors();

    createColorPool(); // needs stage/lights/actors to know number of descriptors
    createColorLayouts();
    createColorSets();     // needs descriptorsetLayout and descriptorpool
    createColorPipeline(); // needs descriptorsetlayout
    createShadowPool();
    createShadowLayouts();
    createShadowSets();
    createShadowPipeline();
}

void Scene::createShadow()
{
    shadow.vulkan = vulkan;
    shadow.mipLevels = 1;
    shadow.layers = 6;
    shadow.numSamples = VK_SAMPLE_COUNT_1_BIT;
    shadow.format = VK_FORMAT_R32_SFLOAT;
    shadow.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    shadow.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    shadow.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    shadow.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    shadow.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    shadow.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    shadow.resize(1024, 1024);

    shadow.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadow.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadow.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    shadow.maxAnisotropy = 1.0F;
    shadow.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    shadow.createSampler();

    stage.backdrop.shadowMap = &shadow;
}

void Scene::createMaterials()
{
    materials.vulkan = vulkan;
    materials.loadConfig(config->materials);
}

void Scene::createStage()
{
    stage.vulkan = vulkan;
    stage.config = &config->stage;
    stage.player = player;
    stage.materials = &materials;
    stage.shadow = &shadow;

    stage.create();
}

void Scene::createLights()
{
    pointLights.resize(numLights);
    for (auto &lightConfig : config->pointLights)
    {
        pointLights[lightConfig.index].config = &lightConfig;
        pointLights[lightConfig.index].vulkan = vulkan;
        pointLights[lightConfig.index].create();
    }
}

void Scene::createActors()
{
    actors.resize(config->actors.size());
    for (auto &actorConfig : config->actors)
    {
        actors[actorConfig.index].config = &actorConfig;
        actors[actorConfig.index].vulkan = vulkan;
        actors[actorConfig.index].materials = &materials;
        actors[actorConfig.index].shadow = &shadow;
        actors[actorConfig.index].create();
    }
}

void Scene::cleanup()
{
    stage.cleanup();
    colorPipeline.cleanup();
    vkDestroyDescriptorPool(vulkan->device, colorPool, nullptr);
}

void Scene::recreate()
{
    player->updateAspectRatio(static_cast<float>(vulkan->width), static_cast<float>(vulkan->height));

    stage.recreate();
    createColorPool();
    createColorSets();
    createColorPipeline();
}

void Scene::drawColor(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    stage.backdrop.draw(commandBuffer, currentImage);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, colorPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : stage.models)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model.vertexBuffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, colorPipeline.pipelineLayout, 0, 1,
                                &model.colorSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, model.indexSize, 1, 0, 0, 0);
    }
    for (auto &actor : actors)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &actor.model.vertexBuffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, actor.model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, colorPipeline.pipelineLayout, 0, 1,
                                &actor.model.colorSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, actor.model.indexSize, 1, 0, 0, 0);
    }
}

void Scene::drawShadow(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : stage.models)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model.vertexBuffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.pipelineLayout, 0, 1,
                                &model.shadowSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, model.indexSize, 1, 0, 0, 0);
    }
    for (auto &actor : actors)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &actor.model.vertexBuffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, actor.model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.pipelineLayout, 0, 1,
                                &actor.model.shadowSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, actor.model.indexSize, 1, 0, 0, 0);
    }
}

void Scene::update(uint32_t currentImage)
{
    stage.backdrop.update(currentImage);

    for (int32_t i = 0; i < numLights; ++i)
    {
        uLight.light[i].position = pointLights[i].light.position;
        //uLight.light[i].position.x += glm::sin(Timer::time());
        uLight.light[i].color = pointLights[i].light.color;
        uLight.light[i].lumens = pointLights[i].light.lumens;
    }

    // uLight.light[0].position = -1.F * player->position;

    UniformShadow shadowmvp{};
    // clip converts perspective to vulkan
    shadowmvp.projection = glm::perspective(glm::radians(90.F), 1.F, vulkan->zNear, vulkan->zFar);
    // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmappingomni/shadowmappingomni.cpp
    // POSITIVE_X
    shadowmvp.view[0] = glm::rotate(glm::mat4(1.F), glm::radians(90.F), glm::vec3(0.F, 1.F, 0.F));
    shadowmvp.view[0] = glm::rotate(shadowmvp.view[0], glm::radians(180.F), glm::vec3(1.F, 0.F, 0.F));
    // NEGATIVE_X
    shadowmvp.view[1] = glm::rotate(glm::mat4(1.F), glm::radians(-90.F), glm::vec3(0.F, 1.F, 0.F));
    shadowmvp.view[1] = glm::rotate(shadowmvp.view[1], glm::radians(180.F), glm::vec3(1.F, 0.F, 0.F));
    // POSITIVE_Y
    shadowmvp.view[2] = glm::rotate(glm::mat4(1.F), glm::radians(-90.F), glm::vec3(1.F, 0.F, 0.F));
    // NEGATIVE_Y
    shadowmvp.view[3] = glm::rotate(glm::mat4(1.F), glm::radians(90.F), glm::vec3(1.F, 0.F, 0.F));
    // POSITIVE_Z
    shadowmvp.view[4] = glm::rotate(glm::mat4(1.F), glm::radians(180.F), glm::vec3(1.F, 0.F, 0.F));
    // NEGATIVE_Z
    shadowmvp.view[5] = glm::rotate(glm::mat4(1.F), glm::radians(180.F), glm::vec3(0.F, 0.F, 1.F));

    glm::vec3 lightPos = pointLights[0].light.position;
    shadowmvp.lightpos = glm::vec4(lightPos, 1.F);

    for (auto &stagemodel : stage.models)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.F), stagemodel.position);
        model = glm::rotate(model, glm::radians(stagemodel.rotation.x), glm::vec3(1.F, 0.F, 0.F));
        model = glm::rotate(model, glm::radians(stagemodel.rotation.y), glm::vec3(0.F, 1.F, 0.F));
        model = glm::rotate(model, glm::radians(stagemodel.rotation.z), glm::vec3(0.F, 0.F, 1.F));
        model = glm::scale(model, stagemodel.scale);

        stagemodel.uTessEval.model = model;
        stagemodel.uTessEval.viewProjection = player->perspective * player->view;

        stagemodel.tescBuffers[currentImage].update(&stagemodel.uTessControl, sizeof(stagemodel.uTessControl));
        stagemodel.teseBuffers[currentImage].update(&stagemodel.uTessEval, sizeof(stagemodel.uTessEval));
        stagemodel.uniformLights[currentImage].update(&uLight, sizeof(uLight));

        shadowmvp.model = glm::translate(model, glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z));
        stagemodel.shadowBuffers[currentImage].update(&shadowmvp, sizeof(shadowmvp));
    }

    for (auto &actor : actors)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0F), actor.model.position);
        model = glm::rotate(model, glm::radians(actor.model.rotation.x), glm::vec3(1.0F, 0.0F, 0.0F));
        model = glm::rotate(model, glm::radians(actor.model.rotation.y), glm::vec3(0.0F, 1.0F, 0.0F));
        model = glm::rotate(model, glm::radians(actor.model.rotation.z), glm::vec3(0.0F, 0.0F, 1.0F));
        model = glm::scale(model, actor.model.scale);

        actor.model.uTessEval.model = model;
        actor.model.uTessEval.viewProjection = player->perspective * player->view;

        actor.model.tescBuffers[currentImage].update(&actor.model.uTessControl, sizeof(actor.model.uTessControl));
        actor.model.teseBuffers[currentImage].update(&actor.model.uTessEval, sizeof(actor.model.uTessEval));
        actor.model.uniformLights[currentImage].update(&uLight, sizeof(uLight));

        shadowmvp.model = glm::translate(model, glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z));
        actor.model.shadowBuffers[currentImage].update(&shadowmvp, sizeof(shadowmvp));
    }
}

void Scene::createColorPool()
{
    auto numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (numTessBuffers() + numUniformLights()) * numSwapChainImages;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = numImageSamplers() * numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = (numTessBuffers() + numUniformLights() + numImageSamplers()) * numSwapChainImages;

    CheckResult(vkCreateDescriptorPool(vulkan->device, &poolInfo, nullptr, &colorPool));
}

void Scene::createColorLayouts()
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
    uLightLayoutBinding.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding shadowLayoutBinding = {};
    shadowLayoutBinding.binding = 4;
    shadowLayoutBinding.descriptorCount = 1;
    shadowLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadowLayoutBinding.pImmutableSamplers = nullptr;
    shadowLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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

    std::array<VkDescriptorSetLayoutBinding, 10> bindings = {
        uTessControlLayoutBinding, uTessEvalLayoutBinding, dispLayoutBinding,   uLightLayoutBinding,
        shadowLayoutBinding,       diffuseLayoutBinding,   normalLayoutBinding, roughnessLayoutBinding,
        metallicLayoutBinding,     aoLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    CheckResult(vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, nullptr, &colorLayout));
}

void Scene::createColorSets()
{
    stage.createColorSets(colorPool, colorLayout);

    for (auto &actor : actors)
    {
        actor.model.createColorSets(colorPool, colorLayout);
    }
}

void Scene::createColorPipeline()
{
    colorPipeline.vulkan = vulkan;
    colorPipeline.descriptorSetLayout = colorLayout;
    colorPipeline.loadDefaults(vulkan->colorPass);

    auto vertShaderCode = readFile("resources/shaders/scene.vert.spv");
    auto tescShaderCode = readFile("resources/shaders/scene.tesc.spv");
    auto teseShaderCode = readFile("resources/shaders/scene.tese.spv");
    auto fragShaderCode = readFile("resources/shaders/scene.frag.spv");

    colorPipeline.vertShaderStageInfo.module = vulkan->createShaderModule(vertShaderCode);
    colorPipeline.tescShaderStageInfo.module = vulkan->createShaderModule(tescShaderCode);
    colorPipeline.teseShaderStageInfo.module = vulkan->createShaderModule(teseShaderCode);
    colorPipeline.fragShaderStageInfo.module = vulkan->createShaderModule(fragShaderCode);

    colorPipeline.shaderStages = {colorPipeline.vertShaderStageInfo, colorPipeline.tescShaderStageInfo,
                                  colorPipeline.teseShaderStageInfo, colorPipeline.fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    colorPipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    colorPipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    colorPipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    colorPipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    colorPipeline.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    colorPipeline.pipelineInfo.pTessellationState = &colorPipeline.tessellationState;
    colorPipeline.create();
}

void Scene::createShadowPool()
{
    auto numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<VkDescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = numShadows() * numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = numShadows() * numSwapChainImages;

    CheckResult(vkCreateDescriptorPool(vulkan->device, &poolInfo, nullptr, &shadowPool));
}

void Scene::createShadowLayouts()
{
    VkDescriptorSetLayoutBinding shadowLayoutBinding = {};
    shadowLayoutBinding.binding = 0;
    shadowLayoutBinding.descriptorCount = 1;
    shadowLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    shadowLayoutBinding.pImmutableSamplers = nullptr;
    shadowLayoutBinding.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> layouts = {shadowLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<int32_t>(layouts.size());
    layoutInfo.pBindings = layouts.data();

    CheckResult(vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, nullptr, &shadowLayout));
}

void Scene::createShadowSets()
{
    stage.createShadowSets(shadowPool, shadowLayout);

    for (auto &actor : actors)
    {
        actor.model.createShadowSets(shadowPool, shadowLayout);
    }
}

void Scene::createShadowPipeline()
{
    shadowPipeline.vulkan = vulkan;
    shadowPipeline.descriptorSetLayout = shadowLayout;
    shadowPipeline.loadDefaults(vulkan->shadowPass);

    auto vertShaderCode = readFile("resources/shaders/shadow.vert.spv");
    auto geomShaderCode = readFile("resources/shaders/shadow.geom.spv");
    auto fragShaderCode = readFile("resources/shaders/shadow.frag.spv");

    shadowPipeline.vertShaderStageInfo.module = vulkan->createShaderModule(vertShaderCode);
    shadowPipeline.geomShaderStageInfo.module = vulkan->createShaderModule(geomShaderCode);
    shadowPipeline.fragShaderStageInfo.module = vulkan->createShaderModule(fragShaderCode);

    shadowPipeline.shaderStages = {shadowPipeline.vertShaderStageInfo, shadowPipeline.geomShaderStageInfo,
                                   shadowPipeline.fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    shadowPipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    shadowPipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    shadowPipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    shadowPipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    shadowPipeline.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    shadowPipeline.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    shadowPipeline.create();
}

} // namespace tat
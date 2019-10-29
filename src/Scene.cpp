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
    createBackdrop();
    createModels();

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

    backdrop.shadowMap = &shadow;
}

void Scene::createMaterials()
{
    materials.vulkan = vulkan;
    materials.loadConfigs(config->materials);
}

void Scene::createBackdrop()
{
    backdrop.vulkan = vulkan;
    backdrop.player = player;
    backdrop.path = config->backdrop;
    backdrop.shadowMap = &shadow;

    backdrop.create();
}

void Scene::createLights()
{
    pointLights.resize(numLights);
    int32_t index = 0;
    for (auto &lightConfig : config->pointLights)
    {
        pointLights[index].config = &lightConfig;
        pointLights[index].vulkan = vulkan;
        pointLights[index].create();
        ++index;
    }
}

void Scene::createModels()
{
    models.resize(config->models.size());
    int32_t index = 0;
    for (auto &actorConfig : config->models)
    {
        models[index].config = &actorConfig;
        models[index].vulkan = vulkan;
        models[index].materials = &materials;
        models[index].shadow = &shadow;
        models[index].create();
        ++index;
    }
}

void Scene::cleanup()
{
    backdrop.cleanup();
    colorPipeline.cleanup();
    vkDestroyDescriptorPool(vulkan->device, colorPool, nullptr);
}

void Scene::recreate()
{
    player->updateAspectRatio(static_cast<float>(vulkan->width), static_cast<float>(vulkan->height));

    backdrop.recreate();
    createColorPool();
    createColorSets();
    createColorPipeline();
}

void Scene::drawColor(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    backdrop.draw(commandBuffer, currentImage);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, colorPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model.vertexBuffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, colorPipeline.pipelineLayout, 0, 1,
                                &model.colorSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, model.indexSize, 1, 0, 0, 0);
    }
}

void Scene::drawShadow(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model.vertexBuffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline.pipelineLayout, 0, 1,
                                &model.shadowSets[currentImage], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, model.indexSize, 1, 0, 0, 0);
    }
}

void Scene::update(uint32_t currentImage)
{
    backdrop.update(currentImage);

    for (int32_t i = 0; i < numLights; ++i)
    {
        uLight.light[i].position = pointLights[i].light.position;
        // uLight.light[i].position.x += glm::sin(Timer::time());
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

    for (auto &model : models)
    {
        glm::mat4 m = glm::translate(glm::mat4(1.F), model.position);
        m = glm::rotate(m, glm::radians(model.rotation.x), glm::vec3(1.F, 0.F, 0.F));
        m = glm::rotate(m, glm::radians(model.rotation.y), glm::vec3(0.F, 1.F, 0.F));
        m = glm::rotate(m, glm::radians(model.rotation.z), glm::vec3(0.F, 0.F, 1.F));
        m = glm::scale(m, model.scale);

        model.uTessEval.model = m;
        model.uTessEval.viewProjection = player->perspective * player->view;

        model.tescBuffers[currentImage].update(&model.uTessControl, sizeof(model.uTessControl));
        model.teseBuffers[currentImage].update(&model.uTessEval, sizeof(model.uTessEval));
        model.uniformLights[currentImage].update(&uLight, sizeof(uLight));

        shadowmvp.model = glm::translate(m, glm::vec3(-lightPos.x, -lightPos.y, -lightPos.z));
        model.shadowBuffers[currentImage].update(&shadowmvp, sizeof(shadowmvp));
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

    for (auto &model : models)
    {
        model.createColorSets(colorPool, colorLayout);
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
    for (auto &model : models)
    {
        model.createShadowSets(shadowPool, shadowLayout);
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
#include "Scene.hpp"
#include "Vulkan.hpp"
#include "helpers.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"

namespace tat
{

Scene::~Scene()
{
    vulkan->device.destroyDescriptorSetLayout(shadowLayout);
    vulkan->device.destroyDescriptorSetLayout(colorLayout);
    vulkan->device.destroyDescriptorPool(colorPool);
    vulkan->device.destroyDescriptorPool(shadowPool);
}

void Scene::create()
{
    createShadow();
    createLights();

    createMaterials();
    createMeshes();
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
    shadow.numSamples = vk::SampleCountFlagBits::e1;
    shadow.format = vk::Format::eR32Sfloat;
    shadow.layout = vk::ImageLayout::eColorAttachmentOptimal;
    shadow.aspect = vk::ImageAspectFlagBits::eColor;
    shadow.imageUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
    shadow.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    shadow.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    shadow.viewType = vk::ImageViewType::eCube;
    shadow.resize(1024, 1024);

    shadow.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    shadow.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    shadow.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    shadow.maxAnisotropy = 1.0F;
    shadow.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    shadow.createSampler();

    backdrop.shadowMap = &shadow;
}

void Scene::createMaterials()
{
    materials.vulkan = vulkan;
    materials.loadConfigs(config->materials);
}

void Scene::createMeshes()
{
    meshes.vulkan = vulkan;
    meshes.loadConfigs(config->meshes);
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
    for (auto &modelConfig : config->models)
    {
        models[index].config = &modelConfig;
        models[index].vulkan = vulkan;
        models[index].materials = &materials;
        models[index].meshes = &meshes;
        models[index].shadow = &shadow;
        models[index].create();
        ++index;
    }
}

void Scene::cleanup()
{
    backdrop.cleanup();
    colorPipeline.cleanup();
    vulkan->device.destroyDescriptorPool(colorPool);
}

void Scene::recreate()
{
    player->updateAspectRatio(static_cast<float>(vulkan->width), static_cast<float>(vulkan->height));

    backdrop.recreate();
    createColorPool();
    createColorSets();
    createColorPipeline();
}

void Scene::drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
    backdrop.draw(commandBuffer, currentImage);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, colorPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        commandBuffer.bindVertexBuffers(0, 1, &model.mesh->vertexBuffer.buffer, offsets.data());
        commandBuffer.bindIndexBuffer(model.mesh->indexBuffer.buffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, colorPipeline.pipelineLayout, 0, 1,
                                         &model.colorSets[currentImage], 0, nullptr);
        commandBuffer.drawIndexed(model.mesh->indexSize, 1, 0, 0, 0);
    }
}

void Scene::drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
     commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        commandBuffer.bindVertexBuffers(0, 1, &model.mesh->vertexBuffer.buffer, offsets.data());
        commandBuffer.bindIndexBuffer(model.mesh->indexBuffer.buffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowPipeline.pipelineLayout, 0, 1,
                                         &model.shadowSets[currentImage], 0, nullptr);
        commandBuffer.drawIndexed(model.mesh->indexSize, 1, 0, 0, 0);
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

    std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = (numTessBuffers() + numUniformLights()) * numSwapChainImages;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = numImageSamplers() * numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = (numTessBuffers() + numUniformLights() + numImageSamplers()) * numSwapChainImages;

    colorPool = vulkan->device.createDescriptorPool(poolInfo);
}

void Scene::createColorLayouts()
{
    vk::DescriptorSetLayoutBinding uTessControlLayoutBinding = {};
    uTessControlLayoutBinding.binding = 0;
    uTessControlLayoutBinding.descriptorCount = 1;
    uTessControlLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uTessControlLayoutBinding.pImmutableSamplers = nullptr;
    uTessControlLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eTessellationControl;

    vk::DescriptorSetLayoutBinding uTessEvalLayoutBinding = {};
    uTessEvalLayoutBinding.binding = 1;
    uTessEvalLayoutBinding.descriptorCount = 1;
    uTessEvalLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uTessEvalLayoutBinding.pImmutableSamplers = nullptr;
    uTessEvalLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eTessellationEvaluation;

    vk::DescriptorSetLayoutBinding dispLayoutBinding = {};
    dispLayoutBinding.binding = 2;
    dispLayoutBinding.descriptorCount = 1;
    dispLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    dispLayoutBinding.pImmutableSamplers = nullptr;
    dispLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eTessellationEvaluation;

    vk::DescriptorSetLayoutBinding uLightLayoutBinding = {};
    uLightLayoutBinding.binding = 3;
    uLightLayoutBinding.descriptorCount = 1;
    uLightLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uLightLayoutBinding.pImmutableSamplers = nullptr;
    uLightLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding shadowLayoutBinding = {};
    shadowLayoutBinding.binding = 4;
    shadowLayoutBinding.descriptorCount = 1;
    shadowLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    shadowLayoutBinding.pImmutableSamplers = nullptr;
    shadowLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding diffuseLayoutBinding = {};
    diffuseLayoutBinding.binding = 5;
    diffuseLayoutBinding.descriptorCount = 1;
    diffuseLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    diffuseLayoutBinding.pImmutableSamplers = nullptr;
    diffuseLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding normalLayoutBinding = {};
    normalLayoutBinding.binding = 6;
    normalLayoutBinding.descriptorCount = 1;
    normalLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    normalLayoutBinding.pImmutableSamplers = nullptr;
    normalLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding roughnessLayoutBinding = {};
    roughnessLayoutBinding.binding = 7;
    roughnessLayoutBinding.descriptorCount = 1;
    roughnessLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    roughnessLayoutBinding.pImmutableSamplers = nullptr;
    roughnessLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding metallicLayoutBinding = {};
    metallicLayoutBinding.binding = 8;
    metallicLayoutBinding.descriptorCount = 1;
    metallicLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    metallicLayoutBinding.pImmutableSamplers = nullptr;
    metallicLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding aoLayoutBinding = {};
    aoLayoutBinding.binding = 9;
    aoLayoutBinding.descriptorCount = 1;
    aoLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    aoLayoutBinding.pImmutableSamplers = nullptr;
    aoLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 10> bindings = {
        uTessControlLayoutBinding, uTessEvalLayoutBinding, dispLayoutBinding,   uLightLayoutBinding,
        shadowLayoutBinding,       diffuseLayoutBinding,   normalLayoutBinding, roughnessLayoutBinding,
        metallicLayoutBinding,     aoLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    colorLayout = vulkan->device.createDescriptorSetLayout(layoutInfo);
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

    colorPipeline.inputAssembly.topology = vk::PrimitiveTopology::ePatchList;

    colorPipeline.pipelineInfo.pTessellationState = &colorPipeline.tessellationState;
    colorPipeline.create();
}

void Scene::createShadowPool()
{
    auto numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = numShadows() * numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = numShadows() * numSwapChainImages;

    shadowPool = vulkan->device.createDescriptorPool(poolInfo);
}

void Scene::createShadowLayouts()
{
    vk::DescriptorSetLayoutBinding shadowLayoutBinding = {};
    shadowLayoutBinding.binding = 0;
    shadowLayoutBinding.descriptorCount = 1;
    shadowLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    shadowLayoutBinding.pImmutableSamplers = nullptr;
    shadowLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eGeometry;

    std::array<vk::DescriptorSetLayoutBinding, 1> layouts = {shadowLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.bindingCount = static_cast<int32_t>(layouts.size());
    layoutInfo.pBindings = layouts.data();

    shadowLayout = vulkan->device.createDescriptorSetLayout(layoutInfo);
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

    shadowPipeline.rasterizer.frontFace = vk::FrontFace::eClockwise;
    shadowPipeline.multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    shadowPipeline.create();
}

} // namespace tat
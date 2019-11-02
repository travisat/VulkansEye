#include "Scene.hpp"
#include "Vulkan.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "helpers.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"

namespace tat
{

Scene::~Scene()
{
    vulkan->device.destroyDescriptorSetLayout(sunLayout);
    vulkan->device.destroyDescriptorSetLayout(shadowLayout);
    vulkan->device.destroyDescriptorSetLayout(colorLayout);
    vulkan->device.destroyDescriptorPool(colorPool);
    vulkan->device.destroyDescriptorPool(shadowPool);
    vulkan->device.destroyDescriptorPool(sunPool);
}

void Scene::create()
{
    createShadow();
    createLights();
    createSun();

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
    createSunPool();
    createSunLayouts();
    createSunSets();
    createSunPipeline();
}

void Scene::createSun()
{
    sun.vulkan = vulkan;
    sun.mipLevels = 1;
    sun.layers = 1;
    sun.format = vulkan->findDepthFormat(); 
    sun.layout = vk::ImageLayout::eUndefined;
    sun.imageUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eDepthStencilAttachment;
    sun.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    sun.aspect = vk::ImageAspectFlagBits::eDepth;
    sun.resize(1024, 1024);

    sun.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    sun.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    sun.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    sun.maxAnisotropy = 1.F;
    sun.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    
    sun.createSampler();
}

void Scene::createShadow()
{
    shadow.vulkan = vulkan;
    shadow.mipLevels = 1;
    shadow.layers = 6;
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
    backdrop.config = &config->backdrop;
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
        models[index].sun = &sun;
        models[index].irradianceMap = &backdrop.irradianceMap;
        models[index].radianceMap = &backdrop.radianceMap;
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

void Scene::drawSun(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, sunPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        commandBuffer.bindVertexBuffers(0, 1, &model.mesh->vertexBuffer.buffer, offsets.data());
        commandBuffer.bindIndexBuffer(model.mesh->indexBuffer.buffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, sunPipeline.pipelineLayout, 0, 1,
                                         &model.sunSets[currentImage], 0, nullptr);
        commandBuffer.drawIndexed(model.mesh->indexSize, 1, 0, 0, 0);
    }
}

void Scene::update(uint32_t currentImage)
{
    backdrop.update(currentImage);

    uLight.sunAngle.x = -cos(glm::radians(Timer::time() / 10.F * 360.F)) * 20.F;
    uLight.sunAngle.y = 20.F; // + sin(glm::radians(Timer::time()/10.F * 360.F)) * 20.F;
    uLight.sunAngle.z = -25.F + sin(glm::radians(Timer::time() / 10.F * 360.F)) * 5.F;

    glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(45.F), 1.F, vulkan->zNear, vulkan->zFar);
    glm::mat4 depthViewMatrix = glm::lookAt(uLight.sunAngle, glm::vec3(0.F), glm::vec3(0, 1, 0));
    glm::mat4 sunVP = depthProjectionMatrix * depthViewMatrix * glm::mat4(1.F);

    uLight.radianceMipLevels = backdrop.radianceMap.mipLevels;
    for (int32_t i = 0; i < numLights; ++i)
    {
        uLight.light[i].position = pointLights[i].light.position;
        uLight.light[i].position.x = -player->position.x;
        uLight.light[i].position.z = -player->position.z;
        uLight.light[i].color = pointLights[i].light.color;
        uLight.light[i].lumens = pointLights[i].light.lumens;
    }

    UniformShadow shadowmvp{};
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

    glm::vec3 lightPos = uLight.light[0].position;
    shadowmvp.lightpos = glm::vec4(lightPos, 1.F);

    for (auto &model : models)
    {
        UniformBuffer vertex{};
        // move model to worldspace
        glm::mat4 m = glm::translate(glm::mat4(1.F), model.position);
        m = glm::rotate(m, glm::radians(model.rotation.x), glm::vec3(1.F, 0.F, 0.F));
        m = glm::rotate(m, glm::radians(model.rotation.y), glm::vec3(0.F, 1.F, 0.F));
        m = glm::rotate(m, glm::radians(model.rotation.z), glm::vec3(0.F, 0.F, 1.F));
        m = glm::scale(m, model.scale);

        // createmvp
        vertex.mvp = player->perspective * player->view * m;

        model.vertexBuffers[currentImage].update(&vertex, sizeof(vertex));
        model.uniformLights[currentImage].update(&uLight, sizeof(uLight));

        // make model for model in lightspace
        shadowmvp.model = glm::translate(glm::mat4(1.F), glm::vec3(-lightPos));
        shadowmvp.model = glm::translate(shadowmvp.model, glm::vec3(model.position));
        shadowmvp.model = glm::rotate(shadowmvp.model, glm::radians(model.rotation.x), glm::vec3(1.F, 0.F, 0.F));
        shadowmvp.model = glm::rotate(shadowmvp.model, glm::radians(model.rotation.y), glm::vec3(0.F, 1.F, 0.F));
        shadowmvp.model = glm::rotate(shadowmvp.model, glm::radians(model.rotation.z), glm::vec3(0.F, 0.F, 1.F));
        shadowmvp.model = glm::scale(shadowmvp.model, model.scale);
        model.shadowBuffers[currentImage].update(&shadowmvp, sizeof(shadowmvp));

        uLight.sunMVP = sunVP * m;
        UniformBuffer sun{};
        sun.mvp = uLight.sunMVP;
        model.sunBuffers[currentImage].update(&sun, sizeof(sun));
    }
}

void Scene::createColorPool()
{
    auto numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    // number of models * uniform buffers * swapchainimages
    poolSizes[0].descriptorCount = models.size() * (2) * numSwapChainImages;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    // number of models * imagesamplers * swapchainimages
    poolSizes[1].descriptorCount = models.size() * 9 * numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // number of models * swapchainimages
    poolInfo.maxSets = models.size() * numSwapChainImages;

    colorPool = vulkan->device.createDescriptorPool(poolInfo);
}

void Scene::createColorLayouts()
{
    std::array<vk::DescriptorSetLayoutBinding, 11> bindings{};

    // UniformBuffer
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;

    // uLight
    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = vk::DescriptorType::eUniformBuffer;
    bindings[1].pImmutableSamplers = nullptr;
    bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // shadow
    bindings[2].binding = 2;
    bindings[2].descriptorCount = 1;
    bindings[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[2].pImmutableSamplers = nullptr;
    bindings[2].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // diffuse
    bindings[3].binding = 3;
    bindings[3].descriptorCount = 1;
    bindings[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[3].pImmutableSamplers = nullptr;
    bindings[3].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // normal
    bindings[4].binding = 4;
    bindings[4].descriptorCount = 1;
    bindings[4].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[4].pImmutableSamplers = nullptr;
    bindings[4].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // roughness
    bindings[5].binding = 5;
    bindings[5].descriptorCount = 1;
    bindings[5].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[5].pImmutableSamplers = nullptr;
    bindings[5].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // metallic
    bindings[6].binding = 6;
    bindings[6].descriptorCount = 1;
    bindings[6].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[6].pImmutableSamplers = nullptr;
    bindings[6].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // ao
    bindings[7].binding = 7;
    bindings[7].descriptorCount = 1;
    bindings[7].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[7].pImmutableSamplers = nullptr;
    bindings[7].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // irradiance
    bindings[8].descriptorCount = 1;
    bindings[8].binding = 8;
    bindings[8].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[8].pImmutableSamplers = nullptr;
    bindings[8].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // radiance
    bindings[9].descriptorCount = 1;
    bindings[9].binding = 9;
    bindings[9].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[9].pImmutableSamplers = nullptr;
    bindings[9].stageFlags = vk::ShaderStageFlagBits::eFragment;

    // sun
    bindings[10].descriptorCount = 1;
    bindings[10].binding = 10;
    bindings[10].descriptorType = vk::DescriptorType::eCombinedImageSampler;
    bindings[10].pImmutableSamplers = nullptr;
    bindings[10].stageFlags = vk::ShaderStageFlagBits::eFragment;

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
    auto fragShaderCode = readFile("resources/shaders/scene.frag.spv");

    colorPipeline.vertShaderStageInfo.module = vulkan->createShaderModule(vertShaderCode);
    colorPipeline.fragShaderStageInfo.module = vulkan->createShaderModule(fragShaderCode);

    colorPipeline.shaderStages = {colorPipeline.vertShaderStageInfo, colorPipeline.fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    colorPipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    colorPipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    colorPipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    colorPipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    colorPipeline.create();
}

void Scene::createShadowPool()
{
    auto numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    // number of models * shadow buffers * swapchainimages
    poolSizes[0].descriptorCount = models.size() * 1 * numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // number of models * swapchainimages
    poolInfo.maxSets = models.size() * numSwapChainImages;

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

    //shadowPipeline.rasterizer.frontFace = vk::FrontFace::eClockwise;
    shadowPipeline.multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    shadowPipeline.create();
}

void Scene::createSunPool()
{
    auto numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    // number of models * uniformBuffers * swapchainimages
    poolSizes[0].descriptorCount = models.size() * 1 * numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // number of models * swapchainimages
    poolInfo.maxSets = models.size() * numSwapChainImages;

    sunPool = vulkan->device.createDescriptorPool(poolInfo);
}

void Scene::createSunLayouts()
{
    vk::DescriptorSetLayoutBinding sunLayoutBinding = {};
    sunLayoutBinding.binding = 0;
    sunLayoutBinding.descriptorCount = 1;
    sunLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    sunLayoutBinding.pImmutableSamplers = nullptr;
    sunLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    std::array<vk::DescriptorSetLayoutBinding, 1> layouts = {sunLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.bindingCount = static_cast<int32_t>(layouts.size());
    layoutInfo.pBindings = layouts.data();

    sunLayout = vulkan->device.createDescriptorSetLayout(layoutInfo);
}

void Scene::createSunSets()
{
    for (auto &model : models)
    {
        model.createSunSets(sunPool, sunLayout);
    }
}

void Scene::createSunPipeline()
{
    sunPipeline.vulkan = vulkan;
    sunPipeline.descriptorSetLayout = sunLayout;
    sunPipeline.loadDefaults(vulkan->sunPass);

    auto vertShaderCode = readFile("resources/shaders/sun.vert.spv");
    auto fragShaderCode = readFile("resources/shaders/sun.frag.spv");

    sunPipeline.vertShaderStageInfo.module = vulkan->createShaderModule(vertShaderCode);
    sunPipeline.fragShaderStageInfo.module = vulkan->createShaderModule(fragShaderCode);

    sunPipeline.shaderStages = {sunPipeline.vertShaderStageInfo, sunPipeline.fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    sunPipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    sunPipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    sunPipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    sunPipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    //sunPipeline.rasterizer.frontFace = vk::FrontFace::eClockwise;
    sunPipeline.multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    sunPipeline.create();
}

} // namespace tat
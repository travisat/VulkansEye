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
    if (shadowLayout)
    {
        vulkan->device.destroyDescriptorSetLayout(shadowLayout);
    }
    if (colorLayout)
    {
        vulkan->device.destroyDescriptorSetLayout(colorLayout);
    }
    if (colorPool)
    {
        vulkan->device.destroyDescriptorPool(colorPool);
    }
    if (shadowPool)
    {
        vulkan->device.destroyDescriptorPool(shadowPool);
    }
}

void Scene::create()
{
    createBrdf();
    createShadow();

    loadBackdrop();
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

void Scene::createBrdf()
{
    brdf.vulkan = vulkan;
    brdf.imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    brdf.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    brdf.loadGLI(vulkan->brdfPath);

    brdf.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    brdf.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    brdf.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    brdf.maxAnisotropy = 1.0F;
    brdf.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    brdf.createSampler();
}

void Scene::createShadow()
{
    shadow.vulkan = vulkan;
    shadow.format = vk::Format::eR32Sfloat;
    shadow.layout = vk::ImageLayout::eColorAttachmentOptimal;
    shadow.imageUsage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
    shadow.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    shadow.aspect = vk::ImageAspectFlagBits::eColor;
    shadow.resize(static_cast<int>(vulkan->shadowSize), static_cast<int>(vulkan->shadowSize));

    shadow.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    shadow.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    shadow.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    shadow.maxAnisotropy = 1.F;
    shadow.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    shadow.createSampler();
}

void Scene::loadBackdrop()
{
    backdrop = backdrops->getBackdrop(config.backdrop);
}

void Scene::createModels()
{
    models.resize(config.models.size());
    int32_t index = 0;
    for (auto &modelConfig : config.models)
    {
        models[index].config = &modelConfig;
        models[index].vulkan = vulkan;
        models[index].materials = materials;
        models[index].meshes = meshes;
        models[index].shadow = &shadow;
        models[index].irradianceMap = &backdrop->irradianceMap;
        models[index].radianceMap = &backdrop->radianceMap;
        models[index].brdf = &brdf;
        models[index].create();
        ++index;
    }
}

void Scene::cleanup()
{
    backdrop->cleanup();
    colorPipeline.cleanup();
    vulkan->device.destroyDescriptorPool(colorPool);
}

void Scene::recreate()
{
    player->updateAspectRatio(static_cast<float>(vulkan->width), static_cast<float>(vulkan->height));

    backdrop->recreate();
    createColorPool();
    createColorSets();
    createColorPipeline();
}

void Scene::drawColor(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
    backdrop->draw(commandBuffer, currentImage);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, colorPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        commandBuffer.bindVertexBuffers(0, 1, &model.mesh->buffers.vertex.buffer, offsets.data());
        commandBuffer.bindIndexBuffer(model.mesh->buffers.index.buffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, colorPipeline.pipelineLayout, 0, 1,
                                         &model.colorSets[currentImage], 0, nullptr);
        commandBuffer.drawIndexed(model.mesh->data.indices.size(), 1, 0, 0, 0);
    }
}

void Scene::drawShadow(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowPipeline.pipeline);

    std::array<VkDeviceSize, 1> offsets = {0};
    for (auto &model : models)
    {
        commandBuffer.bindVertexBuffers(0, 1, &model.mesh->buffers.vertex.buffer, offsets.data());
        commandBuffer.bindIndexBuffer(model.mesh->buffers.index.buffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowPipeline.pipelineLayout, 0, 1,
                                         &model.shadowSets[currentImage], 0, nullptr);
        commandBuffer.drawIndexed(model.mesh->data.indices.size(), 1, 0, 0, 0);
    }
}

void Scene::update(uint32_t currentImage)
{
    backdrop->update(currentImage);

    lightsBuffer.light.position = backdrop->light.light.position;
    lightsBuffer.light.rotation = backdrop->light.light.rotation;
    lightsBuffer.light.color = backdrop->light.light.color;
    lightsBuffer.light.lumens = backdrop->light.light.lumens;
    lightsBuffer.light.steradians = backdrop->light.light.steradians;

    glm::mat4 depthProjectionMatrix = glm::ortho(-30.F, 30.F, -30.F, 30.F, vulkan->zNear, vulkan->zFar);
    glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(lightsBuffer.light.position), glm::vec3(0.F), glm::vec3(0, 1, 0));

    lightsBuffer.radianceMipLevels = backdrop->radianceMap.mipLevels;
    lightsBuffer.shadowSize = vulkan->shadowSize;

    for (auto &model : models)
    {
        // move model to worldspace
        glm::mat4 T = glm::translate(glm::mat4(1.F), model.position);
        glm::mat4 R = glm::rotate(glm::mat4(1.F), glm::radians(model.rotation.x), glm::vec3(1.F, 0.F, 0.F));
        R = glm::rotate(R, glm::radians(model.rotation.y), glm::vec3(0.F, 1.F, 0.F));
        R = glm::rotate(R, glm::radians(model.rotation.z), glm::vec3(0.F, 0.F, 1.F));
        glm::mat4 S = glm::scale(glm::mat4(1.F), model.scale);
        glm::mat4 M = T * R * S;

        // create mvp for player space
        vertexBuffer.model = M;
        vertexBuffer.view = player->view;
        vertexBuffer.projection = player->perspective;
        vertexBuffer.normalMatrix = glm::transpose(glm::inverse(player->perspective * player->view * M));
        vertexBuffer.camPos = glm::vec4(-1.F * player->position, 1.F);

        // create mvp for lightspace
        shadowBuffer.model = M;
        shadowBuffer.view = depthViewMatrix;
        shadowBuffer.projection = depthProjectionMatrix;
        vertexBuffer.lightMVP = depthProjectionMatrix * depthViewMatrix * M;
        model.vertexBuffers[currentImage].update(&vertexBuffer, sizeof(vertexBuffer));
        model.lightsBuffers[currentImage].update(&lightsBuffer, sizeof(lightsBuffer));
        model.shadowBuffers[currentImage].update(&shadowBuffer, sizeof(shadowBuffer));
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

    // brdf pregenned texture
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
    // number of models * uniformBuffers * swapchainimages
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
    shadowLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

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
    auto fragShaderCode = readFile("resources/shaders/shadow.frag.spv");

    shadowPipeline.vertShaderStageInfo.module = vulkan->createShaderModule(vertShaderCode);
    shadowPipeline.fragShaderStageInfo.module = vulkan->createShaderModule(fragShaderCode);

    shadowPipeline.shaderStages = {shadowPipeline.vertShaderStageInfo, shadowPipeline.fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescrption = Vertex::getAttributeDescriptions();
    shadowPipeline.vertexInputInfo.vertexBindingDescriptionCount = 1;
    shadowPipeline.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    shadowPipeline.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescrption.size());
    shadowPipeline.vertexInputInfo.pVertexAttributeDescriptions = attributeDescrption.data();

    shadowPipeline.multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    shadowPipeline.create();
}

} // namespace tat
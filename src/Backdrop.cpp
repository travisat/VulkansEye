#include "Backdrop.hpp"
#include "helpers.h"
#include "vulkan/vulkan.hpp"

namespace tat
{

Backdrop::~Backdrop()
{
    vulkan->device.destroyDescriptorSetLayout(descriptorSetLayout);
    vulkan->device.destroyDescriptorPool(descriptorPool);
}

void Backdrop::create()
{
    loadCubeMap(colorMap, config->colorPath);
    loadCubeMap(radianceMap, config->radiancePath);
    loadCubeMap(irradianceMap, config->irradiancePath);

    createDescriptorPool();
    createDescriptorSetLayouts();
    createPipeline();
    createUniformBuffers();
    createDescriptorSets();
}

void Backdrop::cleanup()
{
    pipeline.cleanup();
    vulkan->device.destroyDescriptorPool(descriptorPool);
}

void Backdrop::recreate()
{
    createPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void Backdrop::loadCubeMap(Image &cubeMap,const std::string& path)
{
    cubeMap.vulkan = vulkan;
    cubeMap.tiling = vk::ImageTiling::eOptimal;
    cubeMap.numSamples = vk::SampleCountFlagBits::e1;
    cubeMap.imageUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    cubeMap.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    cubeMap.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    cubeMap.viewType = vk::ImageViewType::eCube;
    cubeMap.loadGLI(path);

    cubeMap.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    cubeMap.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    cubeMap.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    cubeMap.maxAnisotropy = 1.0F;
    cubeMap.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    cubeMap.createSampler();
}

void Backdrop::draw(vk::CommandBuffer commandBuffer, uint32_t currentImage)
{
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.pipelineLayout, 0, 1,
                                     &descriptorSets[currentImage], 0, nullptr);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);
    commandBuffer.draw(3, 1, 0, 0);
}

void Backdrop::createUniformBuffers()
{
    uniformBuffers.resize(vulkan->swapChainImages.size());
    for (auto &buffer : uniformBuffers)
    {
        buffer.vulkan = vulkan;
        buffer.flags = vk::BufferUsageFlagBits::eUniformBuffer;
        buffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        buffer.memFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        buffer.resize(sizeof(UniformBuffer));

        UniformBuffer uBuffer{};
        memcpy(buffer.mapped, &uBuffer, sizeof(uBuffer));
    }
}

void Backdrop::update(uint32_t currentImage)
{
    //skybox is a quad that fills the screen  
    // https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330
    // by unprojecting the mvp (ie applying the inverse backwards)
    glm::mat4 inverseProjection =  inverse(player->perspective);
    glm::mat4 inverseModelView = transpose(player->view);
    uBuffer.mvp = inverseModelView * inverseProjection; 
    memcpy(uniformBuffers[currentImage].mapped, &uBuffer, sizeof(uBuffer));
}

void Backdrop::createDescriptorPool()
{

    auto numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = numSwapChainImages;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = numSwapChainImages;

    descriptorPool = vulkan->device.createDescriptorPool(poolInfo);
}

void Backdrop::createDescriptorSetLayouts()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = vulkan->device.createDescriptorSetLayout(layoutInfo);
}

void Backdrop::createDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(vulkan->swapChainImages.size(), descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets =  vulkan->device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < vulkan->swapChainImages.size(); i++)
    {

        vk::DescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBuffer);

        vk::DescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = colorMap.imageView;
        imageInfo.sampler = colorMap.sampler;

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {};
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vulkan->device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                            nullptr);
    }
};

void Backdrop::createPipeline()
{
    pipeline.vulkan = vulkan;
    pipeline.descriptorSetLayout = descriptorSetLayout;
    pipeline.loadDefaults(vulkan->colorPass);

    auto vertShaderCode = readFile("resources/shaders/backdrop.vert.spv");
    auto fragShaderCode = readFile("resources/shaders/backdrop.frag.spv");

    pipeline.vertShaderStageInfo.module = vulkan->createShaderModule(vertShaderCode);
    pipeline.fragShaderStageInfo.module = vulkan->createShaderModule(fragShaderCode);

    pipeline.shaderStages = {pipeline.vertShaderStageInfo, pipeline.fragShaderStageInfo};

    pipeline.rasterizer.cullMode = vk::CullModeFlagBits::eFront;

    pipeline.depthStencil.depthTestEnable = VK_FALSE;
    pipeline.depthStencil.depthWriteEnable = VK_FALSE;
    pipeline.depthStencil.depthCompareOp = vk::CompareOp::eNever;
    pipeline.create();
}

} // namespace tat
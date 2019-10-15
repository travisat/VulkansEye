#include "Backdrop.hpp"
#include "helpers.h"

namespace tat
{

Backdrop::~Backdrop()
{
    vkDestroySampler(vulkan->device, sampler, nullptr);
    vkDestroyDescriptorSetLayout(vulkan->device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Backdrop::create()
{
    loadCubeMap();

    createDescriptorPool();
    createDescriptorSetLayouts();
    createPipeline();
    createUniformBuffers();
    createDescriptorSets();
}

void Backdrop::loadCubeMap()
{
    cubeMap.vulkan = vulkan;
    cubeMap.format = VK_FORMAT_B8G8R8A8_UNORM;
    cubeMap.tiling = VK_IMAGE_TILING_OPTIMAL;
    cubeMap.numSamples = VK_SAMPLE_COUNT_1_BIT;
    cubeMap.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    cubeMap.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    cubeMap.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    cubeMap.loadTextureCube(path);

    //convert image so shaders can use it
    cubeMap.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(cubeMap.mipLevels);
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    samplerInfo.maxLod = static_cast<float>(cubeMap.mipLevels);

    if (vkCreateSampler(vulkan->device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler");
    }

    cubeMap.createImageView(VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, 6);
}

void Backdrop::cleanup()
{
    pipeline.cleanup();
    vkDestroyDescriptorPool(vulkan->device, descriptorPool, nullptr);
}

void Backdrop::recreate()
{
    createPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

void Backdrop::draw(VkCommandBuffer commandBuffer, uint32_t currentImage)
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &descriptorSets[currentImage], 0, nullptr);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void Backdrop::createUniformBuffers()
{
    uniformBuffers.resize(vulkan->swapChainImages.size());
    for (auto &buffer : uniformBuffers)
    {
        buffer.vulkan = vulkan;
        buffer.flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        buffer.name = "Backdrop/UBO";

        //uniformBufferObject to store in uniformBuffer
        UniformBuffer uBuffer{};
        uBuffer.projection = player->perspective;
        uBuffer.view = player->view;

        //store and add
        buffer.update(&uBuffer, sizeof(UniformBuffer));
    }
}

void Backdrop::updateUniformBuffer(uint32_t currentImage)
{
    uBuffer.projection = player->perspective;
    uBuffer.view = player->view;
    uniformBuffers[currentImage].update(&uBuffer, sizeof(UniformBuffer));
}

void Backdrop::createDescriptorPool()
{

    uint32_t numSwapChainImages = static_cast<uint32_t>(vulkan->swapChainImages.size());

    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = numSwapChainImages;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = numSwapChainImages;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    // set max set size to which set is larger
    poolInfo.maxSets = numSwapChainImages;

    if (vkCreateDescriptorPool(vulkan->device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool");
    }
    Trace("Created ", name, " descriptor pool at ", Timer::systemTime());
}

void Backdrop::createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(vulkan->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create skybox descriptor set layout");
    }
}

void Backdrop::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(vulkan->swapChainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(vulkan->swapChainImages.size());
    if (vkAllocateDescriptorSets(vulkan->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {

        throw std::runtime_error("Failed to allocate skybox descriptorsets");
    }

    for (size_t i = 0; i < vulkan->swapChainImages.size(); i++)
    {

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBuffer);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = cubeMap.imageView;
        imageInfo.sampler = sampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(vulkan->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
};

void Backdrop::createPipeline()
{
    pipeline.vulkan = vulkan;
    pipeline.descriptorSetLayout = descriptorSetLayout;
    pipeline.vertShaderPath = "resources/shaders/backdrop.vert.spv";
    pipeline.fragShaderPath = "resources/shaders/backdrop.frag.spv";
    pipeline.createBackdrop();
}

} //namespace tat
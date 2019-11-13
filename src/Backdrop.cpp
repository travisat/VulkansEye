#include "Backdrop.hpp"

#include <filesystem>
#include <memory>
#include <utility>


namespace tat
{

void Backdrop::load()
{
    debugLogger = spdlog::get("debugLogger");
    auto& state = State::instance();

    colorMap = loadCubeMap(state["backdrops"][name]["colorPath"]);
    radianceMap = loadCubeMap(state["backdrops"][name]["radiancePath"]);
    irradianceMap = loadCubeMap(state["backdrops"][name]["irradiancePath"]);

    light.x = state["backdrops"][name]["light"]["x"];
    light.y = state["backdrops"][name]["light"]["y"];
    light.z = state["backdrops"][name]["light"]["z"];

    createDescriptorPool();
    createDescriptorSetLayouts();
    createPipeline();
    createUniformBuffers();
    createDescriptorSets();
    loaded = true;
    debugLogger->info("Loaded Backdrop {}", name);
}

Backdrop::~Backdrop()
{
    if (descriptorSetLayout)
    {
        vulkan->device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
    if (descriptorPool)
    {
        vulkan->device.destroyDescriptorPool(descriptorPool);
    }
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

auto Backdrop::loadCubeMap(const std::string &path) -> std::shared_ptr<Image>
{
    auto cubeMap = std::make_shared<Image>(vulkan);
    cubeMap->imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    cubeMap->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    cubeMap->imageInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    cubeMap->imageViewInfo.viewType = vk::ImageViewType::eCube;
    cubeMap->load(path);

    cubeMap->samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    cubeMap->samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    cubeMap->samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    cubeMap->samplerInfo.maxAnisotropy = 1.0F;
    cubeMap->samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    cubeMap->createSampler();
    return cubeMap;
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
    backBuffers.resize(vulkan->swapChainImages.size());
    for (auto &buffer : backBuffers)
    {
        buffer.vulkan = vulkan;
        buffer.flags = vk::BufferUsageFlagBits::eUniformBuffer;
        buffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        buffer.memFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        buffer.resize(sizeof(UniformBack));

        memcpy(buffer.mapped, &backBuffer, sizeof(backBuffer));
    }
}

void Backdrop::update(uint32_t currentImage)
{
    // skybox is a quad that fills the screen
    // https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330
    // by unprojecting the mvp (ie applying the inverse backwards)
    glm::mat4 inverseProjection = inverse(camera->projection());
    glm::mat4 inverseModelView = transpose(camera->view());
    backBuffer.inverseMVP = inverseModelView * inverseProjection;
    memcpy(backBuffers[currentImage].mapped, &backBuffer, sizeof(backBuffer));
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
    vk::DescriptorSetLayoutBinding vertexLayoutBinding = {};
    vertexLayoutBinding.binding = 0;
    vertexLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    vertexLayoutBinding.descriptorCount = 1;
    vertexLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    vertexLayoutBinding.pImmutableSamplers = nullptr;

    vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {vertexLayoutBinding, samplerLayoutBinding};

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

    descriptorSets = vulkan->device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < vulkan->swapChainImages.size(); i++)
    {

        vk::DescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = backBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBack);

        vk::DescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = colorMap->imageView;
        imageInfo.sampler = colorMap->sampler;

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

    auto vertPath = "assets/shaders/backdrop.vert.spv";
    auto fragPath = "assets/shaders/backdrop.frag.spv";

    pipeline.vertShaderStageInfo.module = vulkan->createShaderModule(vertPath);
    pipeline.fragShaderStageInfo.module = vulkan->createShaderModule(fragPath);

    pipeline.shaderStages = {pipeline.vertShaderStageInfo, pipeline.fragShaderStageInfo};

    // pipeline.rasterizer.cullMode = vk::CullModeFlagBits::eFront;

    pipeline.depthStencil.depthTestEnable = VK_FALSE;
    pipeline.depthStencil.depthWriteEnable = VK_FALSE;
    pipeline.depthStencil.depthCompareOp = vk::CompareOp::eNever;
    pipeline.create();
}

} // namespace tat
#include <filesystem>
#include <memory>
#include <utility>

#include "Backdrop.hpp"
#include "State.hpp"

namespace tat
{

void Backdrop::load()
{
    auto &backdrop = State::instance().at("backdrops").at(name);

    colorMap = loadCubeMap(backdrop.at("color").get<std::string>());
    radianceMap = loadCubeMap(backdrop.at("radiance").get<std::string>());
    irradianceMap = loadCubeMap(backdrop.at("irradiance").get<std::string>());

    light.x = backdrop.at("light").at(0);
    light.y = backdrop.at("light").at(1);
    light.z = backdrop.at("light").at(2);

    createDescriptorPool();
    createDescriptorSetLayouts();
    createPipeline();
    createUniformBuffers();
    createDescriptorSets();
    loaded = true;
    spdlog::info("Loaded Backdrop {}", name);
}

Backdrop::~Backdrop()
{
    auto &state = State::instance();
    if (descriptorSetLayout)
    {
        state.vulkan->device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
    if (descriptorPool)
    {
        state.vulkan->device.destroyDescriptorPool(descriptorPool);
    }
}

void Backdrop::cleanup()
{
    auto &state = State::instance();
    pipeline.cleanup();
    state.vulkan->device.destroyDescriptorPool(descriptorPool);
}

void Backdrop::recreate()
{
    createPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

auto Backdrop::loadCubeMap(const std::string &file) -> std::shared_ptr<Image>
{
    auto path = State::instance().at("settings").at("backdropsPath").get<std::string>();
    path = path + name + "/" + file;

    auto cubeMap = std::make_shared<Image>();
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
    auto &state = State::instance();
    backBuffers.resize(state.vulkan->swapChainImages.size());
    for (auto &buffer : backBuffers)
    {
        buffer.flags = vk::BufferUsageFlagBits::eUniformBuffer;
        buffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        buffer.memFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        buffer.resize(sizeof(UniformBack));

        memcpy(buffer.mapped, &backBuffer, sizeof(backBuffer));
    }
}

void Backdrop::update(uint32_t currentImage)
{
    auto &state = State::instance();
    // skybox is a quad that fills the screen
    // https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330
    // by unprojecting the mvp (ie applying the inverse backwards)
    glm::mat4 inverseProjection = inverse(state.camera->projection());
    glm::mat4 inverseModelView = transpose(state.camera->view());
    backBuffer.inverseMVP = inverseModelView * inverseProjection;
    memcpy(backBuffers[currentImage].mapped, &backBuffer, sizeof(backBuffer));
}

void Backdrop::createDescriptorPool()
{
    auto &state = State::instance();

    auto numSwapChainImages = static_cast<uint32_t>(state.vulkan->swapChainImages.size());

    std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = numSwapChainImages;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = numSwapChainImages;

    descriptorPool = state.vulkan->device.createDescriptorPool(poolInfo);
}

void Backdrop::createDescriptorSetLayouts()
{
    auto &state = State::instance();
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

    descriptorSetLayout = state.vulkan->device.createDescriptorSetLayout(layoutInfo);
}

void Backdrop::createDescriptorSets()
{
     auto& state = State::instance();
    std::vector<vk::DescriptorSetLayout> layouts(state.vulkan->swapChainImages.size(), descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(state.vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets = state.vulkan->device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < state.vulkan->swapChainImages.size(); i++)
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

        state.vulkan->device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                            nullptr);
    }
};

void Backdrop::createPipeline()
{
     auto& state = State::instance();
    pipeline.descriptorSetLayout = descriptorSetLayout;
    pipeline.loadDefaults(state.vulkan->colorPass);

    auto vertPath = "assets/shaders/backdrop.vert.spv";
    auto fragPath = "assets/shaders/backdrop.frag.spv";

    pipeline.vertShaderStageInfo.module = state.vulkan->createShaderModule(vertPath);
    pipeline.fragShaderStageInfo.module = state.vulkan->createShaderModule(fragPath);

    pipeline.shaderStages = {pipeline.vertShaderStageInfo, pipeline.fragShaderStageInfo};

    // pipeline.rasterizer.cullMode = vk::CullModeFlagBits::eFront;

    pipeline.depthStencil.depthTestEnable = VK_FALSE;
    pipeline.depthStencil.depthWriteEnable = VK_FALSE;
    pipeline.depthStencil.depthCompareOp = vk::CompareOp::eNever;
    pipeline.create();
}

} // namespace tat
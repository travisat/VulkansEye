#include "Backdrop.hpp"
#include "State.hpp"
#include "glm/ext/scalar_constants.hpp"

#include <filesystem>
#include <memory>
#include <utility>

#include <spdlog/spdlog.h>

namespace tat
{

void Backdrop::load()
{
    auto &backdrop = State::instance().at("backdrops").at(name);

    loadCubeMap(backdrop.at("color").get<std::string>(), &colorMap);
    loadCubeMap(backdrop.at("radiance").get<std::string>(), &radianceMap);
    loadCubeMap(backdrop.at("irradiance").get<std::string>(), &irradianceMap);

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
    auto &engine = State::instance().engine;

    colorMap.destroy();
    radianceMap.destroy();
    irradianceMap.destroy();

    if (descriptorSetLayout)
    {
        engine.device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
    if (descriptorPool)
    {
        engine.device.destroyDescriptorPool(descriptorPool);
    }
    pipeline.destroy();
    spdlog::info("Destroyed Backdrop {}", name);
}

void Backdrop::recreate()
{
    auto &engine = State::instance().engine;

    pipeline.destroy();
    createPipeline();
    createUniformBuffers();
    engine.device.destroyDescriptorPool(descriptorPool);
    createDescriptorPool();
    createDescriptorSets();
}

void Backdrop::loadCubeMap(const std::string &file, Image *image)
{
    auto path = State::instance().at("settings").at("backdropsPath").get<std::string>();
    path = path + name + "/" + file;

    image->imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    image->memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image->imageInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    image->imageViewInfo.viewType = vk::ImageViewType::eCube;
    image->load(path);

    image->samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    image->samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    image->samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    image->samplerInfo.maxAnisotropy = 1.0F;
    image->samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    image->createSampler();
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
    auto &engine = State::instance().engine;
    backBuffers.resize(engine.swapChain.count);
    for (auto &buffer : backBuffers)
    {
        buffer.flags = vk::BufferUsageFlagBits::eUniformBuffer;
        buffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        buffer.memFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        buffer.create(sizeof(UniformBack));

        memcpy(buffer.mapped, &backBuffer, sizeof(backBuffer));
    }
}

void Backdrop::update(uint32_t currentImage)
{
    auto &camera = State::instance().camera;
    // skybox is a quad that fills the screen
    // https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330
    // by unprojecting the mvp (ie applying the inverse backwards)
    glm::mat4 inverseProjection = inverse(camera.projection());
    glm::mat4 inverseModelView = transpose(camera.view());
    backBuffer.inverseMVP = inverseModelView * inverseProjection;
    memcpy(backBuffers[currentImage].mapped, &backBuffer, sizeof(backBuffer));
}

void Backdrop::createDescriptorPool()
{
    auto &engine = State::instance().engine;

    auto numSwapChainImages = engine.swapChain.count;

    std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = numSwapChainImages;
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = numSwapChainImages;

    vk::DescriptorPoolCreateInfo poolInfo = {};
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = numSwapChainImages;

    descriptorPool = engine.device.createDescriptorPool(poolInfo);
}

void Backdrop::createDescriptorSetLayouts()
{
    auto &engine = State::instance().engine;
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
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = engine.device.createDescriptorSetLayout(layoutInfo);
}

void Backdrop::createDescriptorSets()
{
    auto &engine = State::instance().engine;
    std::vector<vk::DescriptorSetLayout> layouts(engine.swapChain.count, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = engine.swapChain.count;
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets = engine.device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < engine.swapChain.count; i++)
    {
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = backBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBack);

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = colorMap.imageView;
        imageInfo.sampler = colorMap.sampler;

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
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

        engine.device.updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
};

void Backdrop::createPipeline()
{
    auto &engine = State::instance().engine;
    pipeline.descriptorSetLayout = &descriptorSetLayout;

    auto vertPath = "assets/shaders/backdrop.vert.spv";
    auto fragPath = "assets/shaders/backdrop.frag.spv";
    pipeline.vertShader = engine.createShaderModule(vertPath);
    pipeline.fragShader = engine.createShaderModule(fragPath);

    pipeline.loadDefaults(engine.colorPass.renderPass);
    pipeline.shaderStages = {pipeline.vertShaderStageInfo, pipeline.fragShaderStageInfo};

    pipeline.depthStencil.depthTestEnable = VK_FALSE;
    pipeline.depthStencil.depthWriteEnable = VK_FALSE;
    pipeline.depthStencil.depthCompareOp = vk::CompareOp::eNever;
    pipeline.create();
}

} // namespace tat
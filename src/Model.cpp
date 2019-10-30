#include "Model.hpp"
#include "Vulkan.hpp"
#include "helpers.h"
#include "vulkan/vulkan.hpp"

namespace tat
{

void Model::create()
{
    position = config->position;
    rotation = config->rotation;
    scale = config->scale;
    uTessControl.tessLevel = config->tessLevel;
    uTessEval.tessStrength = config->tessStregth;
    uTessEval.tessAlpha = config->tessAlpha;

    material = materials->getMaterial(config->material);
    mesh = meshes->getMesh(config->mesh);

    createUniformBuffers();
}

void Model::createColorSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout)
{
    std::vector<vk::DescriptorSetLayout> layouts(vulkan->swapChainImages.size(), layout);
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    colorSets = vulkan->device.allocateDescriptorSets(allocInfo);
    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        vk::DescriptorBufferInfo tessControlInfo = {};
        tessControlInfo.buffer = tescBuffers[i].buffer;
        tessControlInfo.offset = 0;
        tessControlInfo.range = sizeof(TessControl);

        vk::DescriptorBufferInfo tessEvalInfo = {};
        tessEvalInfo.buffer = teseBuffers[i].buffer;
        tessEvalInfo.offset = 0;
        tessEvalInfo.range = sizeof(TessEval);

        vk::DescriptorImageInfo dispInfo = {};
        dispInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        dispInfo.imageView = material->displacement.imageView;
        dispInfo.sampler = material->displacement.sampler;

        vk::DescriptorBufferInfo lightInfo = {};
        lightInfo.buffer = uniformLights[i].buffer;
        lightInfo.offset = 0;
        lightInfo.range = sizeof(UniformLight);

        vk::DescriptorImageInfo shadowInfo = {};
        shadowInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        shadowInfo.imageView = shadow->imageView;
        shadowInfo.sampler = shadow->sampler;

        vk::DescriptorImageInfo diffuseInfo = {};
        diffuseInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        diffuseInfo.imageView = material->diffuse.imageView;
        diffuseInfo.sampler = material->diffuse.sampler;

        vk::DescriptorImageInfo normalInfo = {};
        normalInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        normalInfo.imageView = material->normal.imageView;
        normalInfo.sampler = material->normal.sampler;

        vk::DescriptorImageInfo roughnessInfo = {};
        roughnessInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        roughnessInfo.imageView = material->roughness.imageView;
        roughnessInfo.sampler = material->roughness.sampler;

        vk::DescriptorImageInfo metallicInfo = {};
        metallicInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        metallicInfo.imageView = material->metallic.imageView;
        metallicInfo.sampler = material->metallic.sampler;

        vk::DescriptorImageInfo aoInfo = {};
        aoInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        aoInfo.imageView = material->ao.imageView;
        aoInfo.sampler = material->ao.sampler;

        std::array<vk::WriteDescriptorSet, 10> descriptorWrites = {};

        descriptorWrites[0].dstSet = colorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &tessControlInfo;

        descriptorWrites[1].dstSet = colorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &tessEvalInfo;

        descriptorWrites[2].dstSet = colorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &dispInfo;

        descriptorWrites[3].dstSet = colorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &lightInfo;

        descriptorWrites[4].dstSet = colorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &shadowInfo;

        descriptorWrites[5].dstSet = colorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pImageInfo = &diffuseInfo;

        descriptorWrites[6].dstSet = colorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pImageInfo = &normalInfo;

        descriptorWrites[7].dstSet = colorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[7].descriptorCount = 1;
        descriptorWrites[7].pImageInfo = &roughnessInfo;

        descriptorWrites[8].dstSet = colorSets[i];
        descriptorWrites[8].dstBinding = 8;
        descriptorWrites[8].dstArrayElement = 0;
        descriptorWrites[8].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[8].descriptorCount = 1;
        descriptorWrites[8].pImageInfo = &metallicInfo;

        descriptorWrites[9].dstSet = colorSets[i];
        descriptorWrites[9].dstBinding = 9;
        descriptorWrites[9].dstArrayElement = 0;
        descriptorWrites[9].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[9].descriptorCount = 1;
        descriptorWrites[9].pImageInfo = &aoInfo;

        vulkan->device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                            nullptr);
    }
}

void Model::createShadowSets(vk::DescriptorPool pool, vk::DescriptorSetLayout layout)
{
    std::vector<vk::DescriptorSetLayout> layouts(vulkan->swapChainImages.size(), layout);
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    shadowSets = vulkan->device.allocateDescriptorSets(allocInfo);
    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        vk::DescriptorBufferInfo shadowInfo = {};
        shadowInfo.buffer = shadowBuffers[i].buffer;
        shadowInfo.offset = 0;
        shadowInfo.range = sizeof(UniformShadow);

        vk::WriteDescriptorSet shadow{};
        shadow.dstSet = shadowSets[i];
        shadow.dstBinding = 0;
        shadow.dstArrayElement = 0;
        shadow.descriptorType = vk::DescriptorType::eUniformBuffer;
        shadow.descriptorCount = 1;
        shadow.pBufferInfo = &shadowInfo;

        std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {shadow};

        vulkan->device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                            nullptr);
    }
}

void Model::createUniformBuffers()
{
    uniformLights.resize(vulkan->swapChainImages.size());
    tescBuffers.resize(vulkan->swapChainImages.size());
    teseBuffers.resize(vulkan->swapChainImages.size());
    shadowBuffers.resize(vulkan->swapChainImages.size());

    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        tescBuffers[i].vulkan = vulkan;
        tescBuffers[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        tescBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        tescBuffers[i].resize(sizeof(TessControl));

        teseBuffers[i].vulkan = vulkan;
        teseBuffers[i].flags =vk::BufferUsageFlagBits::eUniformBuffer;
        teseBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        teseBuffers[i].resize(sizeof(TessEval));

        uniformLights[i].vulkan = vulkan;
        uniformLights[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        uniformLights[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        uniformLights[i].resize(sizeof(UniformLight));

        shadowBuffers[i].vulkan = vulkan;
        shadowBuffers[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        shadowBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        shadowBuffers[i].resize(sizeof(UniformShadow));
    }
}

} // namespace tat
#include "Model.hpp"
#include "Vulkan.hpp"
#include "helpers.hpp"

namespace tat
{

void Model::create()
{
    name = config->name;
    position = config->position;
    rotation = config->rotation;
    scale = config->scale;

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
        vk::DescriptorBufferInfo vertexInfo = {};
        vertexInfo.buffer = vertBuffers[i].buffer;
        vertexInfo.offset = 0;
        vertexInfo.range = sizeof(UniformVert);

        vk::DescriptorBufferInfo fragInfo = {};
        fragInfo.buffer = fragBuffers[i].buffer;
        fragInfo.offset = 0;
        fragInfo.range = sizeof(UniformFrag);

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

        vk::DescriptorImageInfo irradianceInfo = {};
        irradianceInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        irradianceInfo.imageView = irradianceMap->imageView;
        irradianceInfo.sampler = irradianceMap->sampler;

        vk::DescriptorImageInfo radianceInfo = {};
        radianceInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        radianceInfo.imageView = radianceMap->imageView;
        radianceInfo.sampler = radianceMap->sampler;

        vk::DescriptorImageInfo brdfInfo = {};
        brdfInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        brdfInfo.imageView = brdf->imageView;
        brdfInfo.sampler = brdf->sampler;

        std::array<vk::WriteDescriptorSet, 11> descriptorWrites = {};

        // vert uniform buffer
        descriptorWrites[0].dstSet = colorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &vertexInfo;

        // frag uniform buffer
        descriptorWrites[1].dstSet = colorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &fragInfo;

        // shadow
        descriptorWrites[2].dstSet = colorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &shadowInfo;

        // diffuse
        descriptorWrites[3].dstSet = colorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &diffuseInfo;

        // normal
        descriptorWrites[4].dstSet = colorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &normalInfo;

        // roughness
        descriptorWrites[5].dstSet = colorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pImageInfo = &roughnessInfo;

        // metallic
        descriptorWrites[6].dstSet = colorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pImageInfo = &metallicInfo;

        // ao
        descriptorWrites[7].dstSet = colorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[7].descriptorCount = 1;
        descriptorWrites[7].pImageInfo = &aoInfo;

        // irradiance
        descriptorWrites[8].dstSet = colorSets[i];
        descriptorWrites[8].dstBinding = 8;
        descriptorWrites[8].dstArrayElement = 0;
        descriptorWrites[8].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[8].descriptorCount = 1;
        descriptorWrites[8].pImageInfo = &irradianceInfo;

        // radiance
        descriptorWrites[9].dstSet = colorSets[i];
        descriptorWrites[9].dstBinding = 9;
        descriptorWrites[9].dstArrayElement = 0;
        descriptorWrites[9].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[9].descriptorCount = 1;
        descriptorWrites[9].pImageInfo = &radianceInfo;

        // pregenned brdf sampler
        descriptorWrites[10].dstSet = colorSets[i];
        descriptorWrites[10].dstBinding = 10;
        descriptorWrites[10].dstArrayElement = 0;
        descriptorWrites[10].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[10].descriptorCount = 1;
        descriptorWrites[10].pImageInfo = &brdfInfo;

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
        shadowInfo.buffer = shadBuffers[i].buffer;
        shadowInfo.offset = 0;
        shadowInfo.range = sizeof(UniformShad);

        std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};

        // shadow
        descriptorWrites[0].dstSet = shadowSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &shadowInfo;

        vulkan->device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                            nullptr);
    }
}

void Model::createUniformBuffers()
{
    vertBuffers.resize(vulkan->swapChainImages.size());
    fragBuffers.resize(vulkan->swapChainImages.size());
    shadBuffers.resize(vulkan->swapChainImages.size());

    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        vertBuffers[i].vulkan = vulkan;
        vertBuffers[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        vertBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        vertBuffers[i].resize(sizeof(UniformVert));

        fragBuffers[i].vulkan = vulkan;
        fragBuffers[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        fragBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        fragBuffers[i].resize(sizeof(UniformFrag));

        shadBuffers[i].vulkan = vulkan;
        shadBuffers[i].flags = vk::BufferUsageFlagBits::eUniformBuffer;
        shadBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        shadBuffers[i].resize(sizeof(UniformShad));
    }
}

} // namespace tat
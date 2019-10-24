#include "Model.hpp"
#include "helpers.h"

namespace tat
{

Model::~Model()
{
    vkDestroySampler(vulkan->device, diffuseSampler, nullptr);
    vkDestroySampler(vulkan->device, normalSampler, nullptr);
    vkDestroySampler(vulkan->device, roughnessSampler, nullptr);
    vkDestroySampler(vulkan->device, metallicSampler, nullptr);
    vkDestroySampler(vulkan->device, aoSampler, nullptr);
    vkDestroySampler(vulkan->device, dispSampler, nullptr);
}

void Model::create()
{
    position = config->position;
    rotation = config->rotation;
    scale = config->scale;
    uTessControl.tessLevel = config->tessLevel;
    uTessEval.tessStrength = config->tessStregth;
    uTessEval.tessAlpha = config->tessAlpha;
    createUniformBuffers();

    loadMesh();
    loadMaterial();
}

void Model::loadMesh()
{
    loadObj(config->objPath, vertices, indices);
    vertexSize = static_cast<uint32_t>(vertices.size());
    indexSize = static_cast<uint32_t>(indices.size());
    Trace("Loaded ", config->objPath, " at ", Timer::systemTime());

    // copy buffers to gpu only memory
    Buffer stagingBuffer{};
    stagingBuffer.vulkan = vulkan;
    stagingBuffer.flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    stagingBuffer.update(vertices);
    vertexBuffer.vulkan = vulkan;
    vertexBuffer.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vertexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(vertexBuffer);

    stagingBuffer.update(indices);
    indexBuffer.vulkan = vulkan;
    indexBuffer.flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBuffer.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    stagingBuffer.copyTo(indexBuffer);
}

void Model::loadMaterial()
{
    loadImage(config->diffusePath, diffuse, diffuseSampler);
    loadImage(config->normalPath, normal, normalSampler);
    loadImage(config->roughnessPath, roughness, roughnessSampler);
    loadImage(config->metallicPath, metallic, metallicSampler);
    loadImage(config->aoPath, ao, aoSampler);
    loadImage(config->displacementPath, displacement, dispSampler);
}

void Model::loadImage(const std::string &path, Image &image, VkSampler &sampler)
{
    image.vulkan = vulkan;
    image.imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image.loadSTB(path);

    VkSamplerCreateInfo imageSamplerInfo = {};
    imageSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    imageSamplerInfo.magFilter = VK_FILTER_LINEAR;
    imageSamplerInfo.minFilter = VK_FILTER_LINEAR;
    imageSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    imageSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    imageSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    imageSamplerInfo.anisotropyEnable = VK_TRUE;
    imageSamplerInfo.maxAnisotropy = 16;
    imageSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    imageSamplerInfo.unnormalizedCoordinates = VK_FALSE;
    imageSamplerInfo.compareEnable = VK_FALSE;
    imageSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    imageSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    imageSamplerInfo.maxLod = static_cast<float>(image.mipLevels);

    CheckResult(vkCreateSampler(vulkan->device, &imageSamplerInfo, nullptr, &sampler));
}

void Model::createColorSets(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    std::vector<VkDescriptorSetLayout> layouts(vulkan->swapChainImages.size(), layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    colorSets.resize(vulkan->swapChainImages.size());
    CheckResult(vkAllocateDescriptorSets(vulkan->device, &allocInfo, colorSets.data()));
    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        VkDescriptorBufferInfo tessControlInfo = {};
        tessControlInfo.buffer = tescBuffers[i].buffer;
        tessControlInfo.offset = 0;
        tessControlInfo.range = sizeof(TessControl);

        VkDescriptorBufferInfo tessEvalInfo = {};
        tessEvalInfo.buffer = teseBuffers[i].buffer;
        tessEvalInfo.offset = 0;
        tessEvalInfo.range = sizeof(TessEval);

        VkDescriptorImageInfo dispInfo = {};
        dispInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        dispInfo.imageView = displacement.imageView;
        dispInfo.sampler = dispSampler;

        VkDescriptorBufferInfo lightInfo = {};
        lightInfo.buffer = uniformLights[i].buffer;
        lightInfo.offset = 0;
        lightInfo.range = sizeof(UniformLight);

        VkDescriptorImageInfo shadowInfo = {};
        shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowInfo.imageView = shadow->imageView;
        shadowInfo.sampler = shadowSampler;

        VkDescriptorImageInfo diffuseInfo = {};
        diffuseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        diffuseInfo.imageView = diffuse.imageView;
        diffuseInfo.sampler = diffuseSampler;

        VkDescriptorImageInfo normalInfo = {};
        normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalInfo.imageView = normal.imageView;
        normalInfo.sampler = normalSampler;

        VkDescriptorImageInfo roughnessInfo = {};
        roughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        roughnessInfo.imageView = roughness.imageView;
        roughnessInfo.sampler = roughnessSampler;

        VkDescriptorImageInfo metallicInfo = {};
        metallicInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        metallicInfo.imageView = metallic.imageView;
        metallicInfo.sampler = metallicSampler;

        VkDescriptorImageInfo aoInfo = {};
        aoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        aoInfo.imageView = ao.imageView;
        aoInfo.sampler = aoSampler;

        std::array<VkWriteDescriptorSet, 10> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = colorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &tessControlInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = colorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &tessEvalInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = colorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &dispInfo;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = colorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pBufferInfo = &lightInfo;

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = colorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &shadowInfo;

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = colorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pImageInfo = &diffuseInfo;

        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[6].dstSet = colorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pImageInfo = &normalInfo;

        descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[7].dstSet = colorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[7].descriptorCount = 1;
        descriptorWrites[7].pImageInfo = &roughnessInfo;

        descriptorWrites[8].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[8].dstSet = colorSets[i];
        descriptorWrites[8].dstBinding = 8;
        descriptorWrites[8].dstArrayElement = 0;
        descriptorWrites[8].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[8].descriptorCount = 1;
        descriptorWrites[8].pImageInfo = &metallicInfo;

        descriptorWrites[9].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[9].dstSet = colorSets[i];
        descriptorWrites[9].dstBinding = 9;
        descriptorWrites[9].dstArrayElement = 0;
        descriptorWrites[9].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[9].descriptorCount = 1;
        descriptorWrites[9].pImageInfo = &aoInfo;

        vkUpdateDescriptorSets(vulkan->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                               0, nullptr);
    }
}

void Model::createShadowSets(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    std::vector<VkDescriptorSetLayout> layouts(vulkan->swapChainImages.size(), layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(vulkan->swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    shadowSets.resize(vulkan->swapChainImages.size());
    CheckResult(vkAllocateDescriptorSets(vulkan->device, &allocInfo, shadowSets.data()));
    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {

        VkDescriptorBufferInfo uboInfo = {};
        uboInfo.buffer = uniformBuffer.buffer;
        uboInfo.offset = 0;
        uboInfo.range = sizeof(uniformBuffer);

        VkDescriptorBufferInfo shadowInfo = {};
        shadowInfo.buffer = shadowBuffer.buffer;
        shadowInfo.offset = 0;
        shadowInfo.range = sizeof(shadowTransforms);

        VkDescriptorBufferInfo lightInfo = {};
        lightInfo.buffer = uniformLights[0].buffer;
        lightInfo.offset = 0;
        lightInfo.range = sizeof(UniformLight);

        VkWriteDescriptorSet uboSet{};
        uboSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboSet.dstSet = shadowSets[i];
        uboSet.dstBinding = 0;
        uboSet.dstArrayElement = 0;
        uboSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboSet.descriptorCount = 1;
        uboSet.pBufferInfo = &uboInfo;

        VkWriteDescriptorSet shadow{};
        shadow.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        shadow.dstSet = shadowSets[i];
        shadow.dstBinding = 1;
        shadow.dstArrayElement = 0;
        shadow.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        shadow.descriptorCount = 1;
        shadow.pBufferInfo = &shadowInfo;

        VkWriteDescriptorSet light{};
        light.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        light.dstSet = shadowSets[i];
        light.dstBinding = 2;
        light.dstArrayElement = 0;
        light.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        light.descriptorCount = 1;
        light.pBufferInfo = &lightInfo;

        std::vector<VkWriteDescriptorSet> descriptorWrites = {uboSet, shadow, light};

        vkUpdateDescriptorSets(vulkan->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                               0, nullptr);
    }
}

void Model::createUniformBuffers()
{
    uniformLights.resize(vulkan->swapChainImages.size());
    tescBuffers.resize(vulkan->swapChainImages.size());
    teseBuffers.resize(vulkan->swapChainImages.size());

    for (size_t i = 0; i < vulkan->swapChainImages.size(); ++i)
    {
        tescBuffers[i].vulkan = vulkan;
        tescBuffers[i].flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        tescBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        tescBuffers[i].resize(sizeof(TessControl));

        teseBuffers[i].vulkan = vulkan;
        teseBuffers[i].flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        teseBuffers[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        teseBuffers[i].resize(sizeof(TessEval));

        uniformLights[i].vulkan = vulkan;
        uniformLights[i].flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        uniformLights[i].memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        uniformLights[i].resize(sizeof(UniformLight));
    }

    shadowBuffer.vulkan = vulkan;
    shadowBuffer.flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    shadowBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    shadowBuffer.resize(sizeof(shadowTransforms));

    uniformBuffer.vulkan = vulkan;
    uniformBuffer.flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    uniformBuffer.memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    uniformBuffer.resize(sizeof(UniformBuffer));
}
} // namespace tat
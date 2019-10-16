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
    uTessControl.tessLevel = config->tessLevel;
    uTessEval.tessStrength = config->tessStregth;
    uTessEval.tessAlpha = config->tessAlpha;
    loadMesh();
    loadMaterial();
}

void Model::loadMesh()
{
    loadObj(config->objPath, vertices, indices);
    vertexSize = static_cast<uint32_t>(vertices.size());
    indexSize = static_cast<uint32_t>(indices.size());
    Trace("Loaded ", config->objPath, " at ", Timer::systemTime());
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

    image.generateMipmaps();
    image.createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
   
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

} //namespace tat
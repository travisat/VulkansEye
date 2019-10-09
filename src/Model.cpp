#include "Model.hpp"
#include "helpers.h"

Model::~Model()
{
    vkDestroySampler(vulkan->device, diffuseSampler, nullptr);
    vkDestroySampler(vulkan->device, normalSampler, nullptr);
    vkDestroySampler(vulkan->device, roughnessSampler, nullptr);
    vkDestroySampler(vulkan->device, ambientOcclusionSampler, nullptr);
}

void Model::create()
{
    loadMesh();
    loadMaterial();
}

void Model::loadMesh()
{
    loadObj(config->objPath, vertices, indices, scale, position);
    vertexSize = static_cast<uint32_t>(vertices.size());
    indexSize = static_cast<uint32_t>(indices.size());
    Trace("Loaded ", config->objPath, " at ", Timer::systemTime());
}

void Model::loadMaterial()
{
    loadImage(config->diffusePath, config->imageType, diffuse, VK_FORMAT_R8G8B8A8_UNORM, diffuseSampler);
    loadImage(config->normalPath, config->imageType, normal, VK_FORMAT_R8G8B8A8_UNORM, normalSampler);
    loadImage(config->roughnessPath, config->imageType, roughness, VK_FORMAT_R8_UNORM, roughnessSampler);
    loadImage(config->ambientOcclusionPath, config->imageType, ambientOcclusion, VK_FORMAT_R8_UNORM, ambientOcclusionSampler);
}

void Model::loadImage(const std::string &path, ImageType type, Image &image, VkFormat format, VkSampler &sampler)
{
    image.vulkan = vulkan;
    image.format = format;
    image.imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image.memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    image.type = ImageType::png;
    image.loadFile(path);

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
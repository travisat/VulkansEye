#include "Material.hpp"

Material::Material(State *state, MaterialConfig const &config)
{
    this->state = state;
    this->diffusePath = config.diffusePath;
    this->normalPath = config.normalPath;
    this->id = config.id;
}

Material::~Material()
{
    delete diffuse;
    vkDestroySampler(state->device, diffuseSampler, nullptr);
    delete normal;
    vkDestroySampler(state->device, normalSampler, nullptr);
}

void Material::load()
{

    diffuse = new Image(state, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VMA_MEMORY_USAGE_GPU_ONLY, VK_NULL_HANDLE, diffusePath);

    diffuse->generateMipmaps();

    diffuse->createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

    VkSamplerCreateInfo diffuseSamplerInfo = {};
    diffuseSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    diffuseSamplerInfo.magFilter = VK_FILTER_LINEAR;
    diffuseSamplerInfo.minFilter = VK_FILTER_LINEAR;
    diffuseSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    diffuseSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    diffuseSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    diffuseSamplerInfo.anisotropyEnable = VK_TRUE;
    diffuseSamplerInfo.maxAnisotropy = 16;
    diffuseSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    diffuseSamplerInfo.unnormalizedCoordinates = VK_FALSE;
    diffuseSamplerInfo.compareEnable = VK_FALSE;
    diffuseSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    diffuseSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    diffuseSamplerInfo.minLod = 0;
    diffuseSamplerInfo.maxLod = static_cast<float>(diffuse->mipLevels);
    diffuseSamplerInfo.mipLodBias = 0;

    if (vkCreateSampler(state->device, &diffuseSamplerInfo, nullptr, &diffuseSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler");
    }

    normal = new Image(state, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VMA_MEMORY_USAGE_GPU_ONLY, VK_NULL_HANDLE, normalPath);

    normal->generateMipmaps();

    normal->createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

    VkSamplerCreateInfo normalSamplerInfo = {};
    normalSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    normalSamplerInfo.magFilter = VK_FILTER_LINEAR;
    normalSamplerInfo.minFilter = VK_FILTER_LINEAR;
    normalSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    normalSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    normalSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    normalSamplerInfo.anisotropyEnable = VK_TRUE;
    normalSamplerInfo.maxAnisotropy = 16;
    normalSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    normalSamplerInfo.unnormalizedCoordinates = VK_FALSE;
    normalSamplerInfo.compareEnable = VK_FALSE;
    normalSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    normalSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    normalSamplerInfo.maxLod = static_cast<float>(normal->mipLevels);

    if (vkCreateSampler(state->device, &normalSamplerInfo, nullptr, &normalSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler");
    }
}
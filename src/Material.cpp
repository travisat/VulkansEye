#include "Material.hpp"

Material::Material(State *state, MaterialConfig const &config)
{
    this->state = state;
    this->diffusePath = config.diffusePath;
    this->normalPath = config.normalPath;
    this->roughnessPath = config.roughnessPath;
    this->aoPath = config.aoPath;
    this->id = config.id;
}

Material::~Material()
{
    delete diffuse;
    vkDestroySampler(state->device, diffuseSampler, nullptr);
    delete normal;
    vkDestroySampler(state->device, normalSampler, nullptr);
    delete roughness;
    vkDestroySampler(state->device, roughnessSampler, nullptr);
    delete ao;
    vkDestroySampler(state->device, aoSampler, nullptr);
}

void Material::load()
{
    diffuse = loadImage(diffusePath, VK_FORMAT_R8G8B8A8_UNORM,&diffuseSampler);
    normal = loadImage(normalPath, VK_FORMAT_R8G8B8A8_UNORM, &normalSampler);
    roughness = loadImage(roughnessPath, VK_FORMAT_R8_UNORM, &roughnessSampler);
    ao = loadImage(aoPath, VK_FORMAT_R8_UNORM, &aoSampler);
}

Image * Material::loadImage(std::string &path, VkFormat format, VkSampler *sampler)
{
     Image *image = new Image(state, format, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VMA_MEMORY_USAGE_GPU_ONLY, VK_NULL_HANDLE, path);

    image->generateMipmaps();

    image->createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

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
    imageSamplerInfo.maxLod = static_cast<float>(image->mipLevels);

    if (vkCreateSampler(state->device, &imageSamplerInfo, nullptr, sampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler from " + path);
    }
    
    return image;
}
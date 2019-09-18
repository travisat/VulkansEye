#pragma once
#include "Helpers.h"
#include "Buffer.h"
#include "Image.h"

struct Material
{
    State *state;
    Image *textureImage;
    VmaAllocation textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    uint32_t id;
    std::string path;

    Material(State *state, std::string path)
    {
        this->state = state;
        this->path = path;
    };

    void load()
    {
        
        textureImage =  new Image(state, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT,
                             VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                             VMA_MEMORY_USAGE_GPU_ONLY, VK_NULL_HANDLE, path);

        textureImage->generateMipmaps();

        textureImageView = textureImage->createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.maxLod = static_cast<float>(textureImage->getMipLevels());

        if (vkCreateSampler(state->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler");
        }
    };
};
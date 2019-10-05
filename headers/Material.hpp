#pragma once

#include "Buffer.hpp"
#include "Image.hpp"
#include "Light.hpp"
#include "Config.h"


class Material
{
public:
    tat::Vulkan *vulkan = nullptr;

    Image diffuse{};
    Image normal{};
    Image roughness{};
    Image ambientOcclusion{};
    VkSampler diffuseSampler = VK_NULL_HANDLE;
    VkSampler normalSampler = VK_NULL_HANDLE;
    VkSampler roughnessSampler = VK_NULL_HANDLE;
    VkSampler ambientOcclusionSampler = VK_NULL_HANDLE;
    uint32_t id = 0;
    std::string name = "Uknown material";

    ~Material();

    void loadConfig(MaterialConfig const &config);

private:
    void loadImage(const std::string &path, ImageType type, Image &image, VkFormat format, VkSampler &sampler);
};
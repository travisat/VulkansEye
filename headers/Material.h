#include <vulkan/vulkan.h>
#include <string>

#include <vk_mem_alloc.h>

struct Material
{
    std::string texturePath;
    uint32_t id;
    
    VkImage textureImage;
    VmaAllocation textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    uint32_t mipLevels;

    
};
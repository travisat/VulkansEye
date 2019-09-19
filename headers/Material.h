#pragma once
#include "Helpers.h"
#include "Buffer.h"
#include "Image.h"

class Material
{
public:
    State *state;
    Image *textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;
    uint32_t id;
    std::string path;

    Material(State *state, std::string path);

    ~Material();

    void load();
};
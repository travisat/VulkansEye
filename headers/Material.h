#pragma once
#include "Helpers.h"
#include "Buffer.h"
#include "Image.h"

class Material
{
public:
    State *state;
    Image *textureImage;
    VkSampler textureSampler;
    uint32_t id;
    std::string path;

    //reserve id 0 for skybox;
    Material(State *state, std::string path, uint32_t id);

    ~Material();

    void load();
};
#pragma once
#include "Helpers.h"
#include "Buffer.h"
#include "Image.h"
#include "Config.h"

class Material
{
public:
    State *state;
    Image *textureImage;
    VkSampler textureSampler;
    uint32_t id;
    std::string path;

    //reserve id 0 for skybox;
    Material(State *state, MaterialConfig const &config);

    ~Material();

    void load();
};
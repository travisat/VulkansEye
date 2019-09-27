#pragma once

#include "Buffer.h"
#include "Image.h"
#include "Config.h"

class Material
{
public:
    State *state;
    Image *diffuse;
    Image *normal;
    VkSampler diffuseSampler;
    VkSampler normalSampler;
    uint32_t id;
    std::string diffusePath;
    std::string normalPath;

    //reserve id 0 for skybox;
    Material(State *state, MaterialConfig const &config);

    ~Material();

    void load();
};
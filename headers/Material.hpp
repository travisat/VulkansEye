#pragma once

#include "Buffer.hpp"
#include "Image.hpp"
#include "Light.hpp"
#include "Config.h"


class Material
{
public:
    State *state;
    Image *diffuse;
    Image *normal;
    Image *roughness;
    Image *ao;
    VkSampler diffuseSampler;
    VkSampler normalSampler;
    VkSampler roughnessSampler;
    VkSampler aoSampler;
    uint32_t id;
    std::string diffusePath;
    std::string normalPath;
    std::string roughnessPath;
    std::string aoPath;

    //reserve id 0 for skybox;
    Material(State *state, MaterialConfig const &config);

    ~Material();

    void load();

    private:
         Image * loadImage(std::string &path, VkFormat format, VkSampler *sampler);
};
#pragma once

#include "Helpers.h"

struct ModelConfig
{
    uint32_t id;
    uint32_t meshId;
    uint32_t materialId;
    uint32_t xpos;
    uint32_t ypos;
    uint32_t zpos;
};


struct Config
{
    // <id, path>
    std::map<uint32_t, std::string> meshes;
    std::map<uint32_t, std::string> materials;

    struct
    {
        std::string meshPath;
        std::string materialPath;
    } skybox;

    std::vector<ModelConfig> modelConfigs;
};
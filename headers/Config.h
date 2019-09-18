#pragma once

#include "Helpers.h"

struct Config
{
    std::vector<std::string> modelPaths;
    std::vector<std::string> materialPaths;
    std::array<std::string, 6> skyboxTextures;

    std::vector<std::array<uint32_t, 2>> objectIndices;
    std::vector<std::array<uint32_t, 3>> objectPositions;
};
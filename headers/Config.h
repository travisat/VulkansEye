#pragma once

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

enum ModelType
{
    obj,
    gltf
};

struct ModelConfig
{
    uint32_t id = 0;
    ModelType type;
    glm::vec3 position;

    //if type is obj these are needed
    uint32_t meshId = 0;
    uint32_t materialId = 0;
    
    //if gltf provide path
    std::string path = "";
};

struct MaterialConfig
{
    uint32_t id = 0;
    std::string diffusePath = "";
    std::string normalPath = "";
    std::string roughnessPath = "";
};

struct MeshConfig
{
    uint32_t id = 0;
    std::string objPath = "";
};

struct LightConfig
{
    uint32_t id = 0;
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 rotation;
};

struct Config
{
    
    std::vector<MeshConfig> meshes {};
    std::vector<MaterialConfig> materials {};
    std::vector<LightConfig> lights {};

    std::string skybox = "";

    std::vector<ModelConfig> models {};
};
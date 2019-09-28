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
    glm::vec3 scale;

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
    std::string aoPath = "";
    std::string metallicPath = "";
};

struct MeshConfig
{
    uint32_t id = 0;
    std::string objPath = "";
};

struct LightConfig
{
    uint32_t id = 0;
    glm::vec4 light;
    float temperature = 0;
    float lumens = 0;
};

struct CameraConfig
{
    float fieldOfView = 60.0f;
    glm::vec3 position;
    glm::vec3 rotation;
};

struct Config
{
    std::vector<CameraConfig> cameras {}; //TODO cameras[0] is only usable camera atm
    std::vector<MeshConfig> meshes {};
    std::vector<MaterialConfig> materials {};
    std::vector<LightConfig> lights {};

    std::string skybox = "";

    std::vector<ModelConfig> models {};
};
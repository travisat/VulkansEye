#pragma once

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

enum class ModelType
{
    unknown,
    obj,
    gltf
};

enum class ImageType
{
    unknown,
    png, //stb_image.h types
    jpg,
    bmp,
    psd,
    tga,
    gif,
    hdr,
    pic,
    pnm,
    dds, //gli types
    ktx,
    kmg
};

struct MaterialConfig
{
    uint32_t id = 0;
    ImageType type = ImageType::png;
    std::string name = "Unknown Material";
    std::string diffusePath = "";
    std::string normalPath = "";
    std::string roughnessPath = "";
    std::string ambientOcclusionPath = "";
    std::string metallicPath = "";
};

struct MeshConfig
{
    uint32_t id = 0;
    std::string name = "Uknown Mesh";
    std::string objPath = "";
};

struct ModelConfig
{
    uint32_t id = 0;
    ModelType type;
    std::string name = "Unknown Model";
    glm::vec3 position;
    glm::vec3 scale;

    //if gltf provide path
    std::string path = "";

    MaterialConfig materialConfig;
    MeshConfig meshConfig;
};

struct LightConfig
{
    uint32_t id = 0;
    std::string name = "Unknown Light";
    glm::vec4 light;
    float temperature = 0;
    float lumens = 0;
};

struct CameraConfig
{
    std::string name = "Unknown Camera";
    float fieldOfView = 60.0f;
    glm::vec3 position;
    glm::vec3 rotation;
};

struct Config
{
    std::vector<CameraConfig> cameras{}; //TODO cameras[0] is only usable camera atm
    std::vector<LightConfig> lights{};
    std::string skybox = "";
    std::vector<ModelConfig> models{};
};
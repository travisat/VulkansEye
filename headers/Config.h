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
    ImageType imageType;
    std::string diffusePath = "resources/textures/default/diffuse.png";
    std::string normalPath = "resources/textures/default/normal.png";
    std::string roughnessPath = "resources/textures/default/roughness.png";
    std::string metallicPath = "resources/textures/default/metallic.png";
    std::string aoPath = "resources/textures/default/ao.png";
};

struct ModelConfig
{
    uint32_t index;
    std::string name = "Unknown Model";
    ModelType modelType; //Todo load textures through obj
    std::string objPath = "";
    MaterialConfig material;    
};

struct ActorConfig
{
    uint32_t index;
    std::string name = "Unknown Actor";
    glm::vec3 position;
    glm::vec3 scale;
    ModelConfig modelConfig;
};

struct LightConfig
{
    uint32_t index;
    std::string name = "Unknown Light";
    glm::vec3 color;
    glm::vec3 position;
    float temperature = 0;
    float lumens = 0;
    ModelConfig modelConfig;
};

struct PlayerConfig
{
    std::string name = "Unknown Player";
    float fieldOfView = 60.0f;
    glm::vec3 position;
    glm::vec3 rotation;
    float height; //meters 1.0f == 1m
};

struct StageConfig
{
    uint32_t index;
    std::string name = "Unknown Stage";
    glm::vec3 scale = glm::vec3(1.0f);
    ModelConfig modelConfig;
};

struct SceneConfig
{
    uint32_t index;
    PlayerConfig playerConfig;
    StageConfig stageConfig;
    std::vector<LightConfig> lights{};
    std::string backdrop = "";
    std::vector<ActorConfig> actors;
};
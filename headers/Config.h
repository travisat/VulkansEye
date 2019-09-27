#pragma once

struct ModelConfig
{
    uint32_t id;
    uint32_t meshId;
    uint32_t materialId;
    uint32_t xpos;
    uint32_t ypos;
    uint32_t zpos;
};

struct MaterialConfig
{
    uint32_t id;
    std::string diffusePath;
    std::string normalPath;
    std::string roughnessPath;
};

struct MeshConfig
{
    uint32_t id;
    std::string objPath;
};

struct LightConfig
{
    uint32_t id;
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 rotation;
};

struct Config
{
    // <id, path>
    std::vector<MeshConfig> meshes;
    std::vector<MaterialConfig> materials;

    std::string skybox;

    std::vector<ModelConfig> modelConfigs;
};
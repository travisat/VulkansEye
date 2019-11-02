#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <json.hpp>

using json = nlohmann::json;

namespace tat
{

constexpr float PI = 3.1415926F;

struct VulkanConfig
{
    std::string name = "none";
    float zNear = 0.1F;
    float zFar = 1024.F;
    int windowWidth = 1024;
    int windowHeight = 768;
    bool sync = true;
    float shadowSize = 1024.F;
};

struct PlayerConfig
{
    float fieldOfView = 60.F;
    glm::vec3 position{};
    glm::vec3 rotation{};
    float height = 1.7F; // meters 1.0f == 1m
    float mass = 100.F;
    float jumpHeight = 1.F;
    float mouseSensitivity = 25.F;
    float velocityMax = 6.F;
    float timeToReachVMax = 0.6F;
    float timeToStopfromVMax = 0.1F;
};

struct LightConfig
{
    std::string name = "default";
    glm::vec3 position = glm::vec3(-2.F, 3.F, -4.F);
    //glm::vec3 rotation = glm::vec3(0.F); //TODO(travis) implement rotation for spot lights
    float temperature = 6400;
    float lumens = 1600;
    float steradians = 4.F * PI;
};

struct BackropConfig
{
    std::string colorPath = "resources/backdrop/desert/color.dds";
    std::string radiancePath = "resources/backdrop/desert/radiance.dds";
    std::string irradiancePath = "resources/backdrop/desert/irradiance.dds";
    LightConfig light{};
};

struct MaterialConfig
{
    std::string name = "default";
    std::string diffuse = "resources/materials/default/diffuse.png";
    std::string normal = "resources/materials/default/normal.png";
    std::string roughness = "resources/materials/default/roughness.png";
    std::string metallic = "resources/materials/default/metallic.png";
    std::string ao = "resources/materials/default/ao.png";
    std::string displacement = "resources/materials/default/displacement.png";
};

struct MeshConfig
{
    std::string name = "cube";
    std::string path = "resources/models/cube.obj";
};

struct ModelConfig
{
    std::string name = "default";
    std::string mesh = "cube";
    std::string material = "default";
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};
};

struct Config
{
    VulkanConfig vulkan{};
    PlayerConfig player{};
    BackropConfig backdrop{};
    std::vector<LightConfig> lights{};
    std::vector<MaterialConfig> materials{};
    std::vector<MeshConfig> meshes{};
    std::vector<ModelConfig> models{};
};

} // namespace tat
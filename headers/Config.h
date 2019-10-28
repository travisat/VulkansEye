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

struct VulkanConfig
{
    std::string name = "none";
    float zNear = 0.1F;
    float zFar = 1024.F;
    int windowWidth = 1024;
    int windowHeight = 768;
};

struct PlayerConfig
{
    float fieldOfView = 60.F;
    glm::vec3 position {};
    glm::vec3 rotation {};
    float height = 1.7F; // meters 1.0f == 1m
    float mass = 100.F;
    float jumpHeight = 1.F;
    float mouseSensitivity = 25.F;
    float velocityMax = 6.F;
    float timeToReachVMax = 0.6F;
    float timeToStopfromVMax = 0.1F;
};

struct PointLightConfig
{
    int32_t index = 0;
    std::string name = "none";
    glm::vec3 position {};
    float temperature = 0;
    float lumens = 0;
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

struct MaterialsConfig
{
    std::vector<MaterialConfig> material{};
};

struct ModelConfig
{
    int32_t index = 0;
    std::string name = "none";
    glm::vec3 position {};
    glm::vec3 rotation {};
    glm::vec3 scale {};
    std::string object = "resources/models/cube.obj";
    std::string material = "default";
    float tessLevel = 1.F;
    float tessStregth = 0.F;
    float tessAlpha = 0.0F;
};

struct ActorConfig
{
    int32_t index = 0;
    std::string name = "none";
    ModelConfig model {};
};

struct StageConfig
{
    std::string backdrop = "resources/backdrop/nebula.dds";
    std::vector<ModelConfig> models {};
};

struct Config
{
    VulkanConfig vulkan {};
    PlayerConfig player {};
    MaterialsConfig materials {};
    StageConfig stage {};
    std::vector<PointLightConfig> pointLights{};
    std::vector<ActorConfig> actors {};
};

} // namespace tat
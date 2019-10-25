#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <json.hpp>

using json = nlohmann::json;

namespace tat
{

struct Config
{
    int32_t index = 0;
    std::string name = "none";
};

struct VulkanConfig : Config
{
    float zNear;
    float zFar;
};

struct ModelConfig : Config
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    std::string objPath;
    std::string diffusePath;
    std::string normalPath;
    std::string roughnessPath;
    std::string metallicPath;
    std::string aoPath;
    std::string displacementPath;
    float tessLevel;
    float tessStregth;
    float tessAlpha;
};

struct ActorConfig : Config
{
    ModelConfig model;
};

struct PointLightConfig : Config
{
    glm::vec3 position;
    float temperature = 0;
    float lumens = 0;
};

struct PlayerConfig : Config
{
    float fieldOfView;
    glm::vec3 position;
    glm::vec3 rotation;
    float height; // meters 1.0f == 1m
    float mass;
    float jumpHeight;
    float mouseSensitivity;
    float velocityMax;
    float timeToReachVMax;
    float timeToStopfromVMax;
};

struct StageConfig : Config
{
    std::string backdrop = "";
    std::vector<ModelConfig> models;
};

struct SceneConfig : Config
{
    VulkanConfig vulkanConfig;
    PlayerConfig playerConfig;
    StageConfig stageConfig;
    std::vector<PointLightConfig> pointLights{};
    std::vector<ActorConfig> actors;
};

} // namespace tat
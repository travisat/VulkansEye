#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

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

struct ModelConfig : Config
{
    std::string objPath;
    std::string diffusePath;
    std::string normalPath;
    std::string roughnessPath;
    std::string metallicPath;
    std::string aoPath;
    std::string displacementPath;
};

struct ActorConfig : Config
{
    glm::vec3 position;
    glm::vec3 rotation;
    ModelConfig modelConfig;
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
    float height; //meters 1.0f == 1m
    float mass;
    float jForce;
    float mouseSensitivity;
    float velocityMax;
    float timeToReachVMax;
    float timeToStopfromVMax;
};

struct StageConfig : Config
{
    std::string backdrop = "";
    ModelConfig modelConfig;
};

struct SceneConfig : Config
{
    PlayerConfig playerConfig;
    StageConfig stageConfig;
    std::vector<PointLightConfig> pointLights{};
    std::vector<ActorConfig> actors;
    float tessLevel;
    float tessAlpha;
    float tessStregth;
};

static void loadSceneConfig(std::string path, SceneConfig &config)
{
    std::ifstream file(path);
    json j;
    file >> j;

    j.at("index").get_to(config.index);
    j.at("name").get_to(config.name);

    config.actors.resize(j["actors"].size());
    int32_t i = 0;
    for (auto &actorconfig : j.at("actors"))
    {
        config.actors[i].index = i;
        actorconfig.at("name").get_to(config.actors[i].name);
        actorconfig.at("position").at("x").get_to(config.actors[i].position.x);
        actorconfig.at("position").at("y").get_to(config.actors[i].position.y);
        actorconfig.at("position").at("z").get_to(config.actors[i].position.z);
        actorconfig.at("rotation").at("x").get_to(config.actors[i].rotation.x);
        actorconfig.at("rotation").at("y").get_to(config.actors[i].rotation.y);
        actorconfig.at("rotation").at("z").get_to(config.actors[i].rotation.z);
        actorconfig.at("model").at("path").get_to(config.actors[i].modelConfig.objPath);
        actorconfig.at("model").at("diffuse").get_to(config.actors[i].modelConfig.diffusePath);
        actorconfig.at("model").at("normal").get_to(config.actors[i].modelConfig.normalPath);
        actorconfig.at("model").at("roughness").get_to(config.actors[i].modelConfig.roughnessPath);
        actorconfig.at("model").at("metallic").get_to(config.actors[i].modelConfig.metallicPath);
        actorconfig.at("model").at("ao").get_to(config.actors[i].modelConfig.aoPath);
        actorconfig.at("model").at("displacement").get_to(config.actors[i].modelConfig.displacementPath);
        ++i;
    }

    j.at("stage").at("backdrop").get_to(config.stageConfig.backdrop);
    j.at("stage").at("path").get_to(config.stageConfig.modelConfig.objPath);
    j.at("stage").at("diffuse").get_to(config.stageConfig.modelConfig.diffusePath);
    j.at("stage").at("normal").get_to(config.stageConfig.modelConfig.normalPath);
    j.at("stage").at("roughness").get_to(config.stageConfig.modelConfig.roughnessPath);
    j.at("stage").at("metallic").get_to(config.stageConfig.modelConfig.metallicPath);
    j.at("stage").at("ao").get_to(config.stageConfig.modelConfig.aoPath);
    j.at("stage").at("displacement").get_to(config.stageConfig.modelConfig.displacementPath);

    config.pointLights.resize(j["lights"].size());
    i = 0;
    for (auto &lightconfig : j.at("lights"))
    {
        config.pointLights[i].index = i;
        lightconfig.at("name").get_to(config.pointLights[i].name);
        lightconfig.at("position").at("x").get_to(config.pointLights[i].position.x);
        lightconfig.at("position").at("y").get_to(config.pointLights[i].position.y);
        lightconfig.at("position").at("z").get_to(config.pointLights[i].position.z);
        lightconfig.at("temperature").get_to(config.pointLights[i].temperature);
        lightconfig.at("lumens").get_to(config.pointLights[i].lumens);
        ++i;
    }

    j.at("player").at("name").get_to(config.playerConfig.name);
    j.at("player").at("height").get_to(config.playerConfig.height);
    j.at("player").at("mass").get_to(config.playerConfig.mass);
    j.at("player").at("velocityMax").get_to(config.playerConfig.velocityMax);
    j.at("player").at("timeToReachVMax").get_to(config.playerConfig.timeToReachVMax);
    j.at("player").at("timeToStopFromVMax").get_to(config.playerConfig.timeToStopfromVMax);
    j.at("player").at("jForce").get_to(config.playerConfig.jForce);
    j.at("player").at("mouseSensitivity").get_to(config.playerConfig.mouseSensitivity);
    j.at("player").at("fieldOfView").get_to(config.playerConfig.fieldOfView);
    j.at("player").at("position").at("x").get_to(config.playerConfig.position.x);
    j.at("player").at("position").at("y").get_to(config.playerConfig.position.y);
    j.at("player").at("position").at("z").get_to(config.playerConfig.position.z);
    j.at("player").at("rotation").at("x").get_to(config.playerConfig.rotation.x);
    j.at("player").at("rotation").at("y").get_to(config.playerConfig.rotation.y);
    j.at("player").at("rotation").at("z").get_to(config.playerConfig.rotation.z);

    j.at("tesselation").at("level").get_to(config.tessLevel);
    j.at("tesselation").at("strength").get_to(config.tessStregth);
    j.at("tesselation").at("alpha").get_to(config.tessAlpha);
};

} //namespace tat
#pragma once

#include "spdlog/logger.h"
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <spdlog/spdlog.h>

#include "State.hpp"

namespace tat
{

class Config
{
  public:
    Config() = default;
    explicit Config(const std::string &path);
    ~Config() = default;

  private:
    std::shared_ptr<spdlog::logger> logger;

    void loadSettings(const std::string &path);
    void loadPlayer(const std::string &path);
    void loadScene(const std::string &path);
    void loadBackdrops(const std::string &path);
    void loadMaterials(const std::string &path);
    void loadMeshes(const std::string &path);
    void loadModels(const std::string &path);

    // Default configs
    json settings = {{"name", "None"},                               //
                     {"zNear", 0.1},                                 //
                     {"zFar", 256.0},                                //
                     {"FoV", 70},                                    //
                     {"mouseSensitivity", 35},                       //
                     {"window", {{"width", 1024}, {"height", 768}}}, //
                     {"vsync", true},                                //
                     {"shadowSize", 1024},                           //
                     {"brdfPath", "assets/brdf.dds"},                //
                     {"playerConfig", "assets/configs/player.json"}, //
                     {"sceneConfig", "assets/configs/scene.json"},   //
                     {"materialsPath", "assets/materials/"},         //
                     {"meshesPath", "assets/meshes/"},               //
                     {"backdropsPath", "assets/backdrops/"}};        //

    json player = {{"height", 1.7},             //
                   {"mass", 100},               //
                   {"velocityMax", 6.0},        //
                   {"timeToReachVMax", 0.6},    //
                   {"timeToStopFromVMax", 0.1}, //
                   {"jumpHeight", 1.0}};        //

    json scene = {{"backrop", "default"},              //
                  {"models", json::object({"default"})}}; //

    json backdrop = {{"color", "assets/backdrop/default/color.dds"},           //
                     {"radiance", "assets/backdrop/default/radiance.dds"},     //
                     {"irradiance", "assets/backdrop/default/irradiance.dds"}, //
                     {"light", {{"x", -10}, {"y", -15}, {"z", 0}}}};           //

    json material = {{"diffuse", "diffuse.dds"},     //
                     {"normal", "normal.dds"},       //
                     {"metallic", "metallic.dds"},   //
                     {"roughness", "roughness.dds"}, //
                     {"ao", "ao.dds"}};              //

    json mesh = {{"file", "default.glb"},                      //
                 {"size", {{"x", 2}, {"y", 2}, {"z", 2}}}}; //

    json model = {{"mesh", "default"},                              //
                  {"material", "default"},                       //
                  {"mass", 0},                                   //
                  {"position", {{"x", 0}, {"y", 0}, {"z", 0}}}, //
                  {"rotation", {{"x", 0}, {"y", 0}, {"z", 0}}},  //
                  {"scale", {{"x", 1}, {"y", 1}, {"z", 1}}}};    //
};

}; // namespace tat
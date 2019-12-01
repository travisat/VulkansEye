#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace tat
{

class Config
{
  public:
    void create(const std::string &path);

  private:
    static void loadSettings(const std::string &path);
    static void loadPlayer(const std::string &path);
    static void loadScene(const std::string &path);
    void loadBackdrops(const std::string &path);
    void loadMaterials(const std::string &path);
    void loadMeshes(const std::string &path);
    void loadModels(const std::string &path);

    // Default configs
    json settings = {{"zNear", 0.1},                                 //
                     {"zFar", 256.0},                                //
                     {"FoV", 70},                                    //
                     {"mouseSensitivity", 35},                       //
                     {"window", {1024, 768}},                        //
                     {"vsync", true},                                //
                     {"shadowSize", 1024},                           //
                     {"brdfPath", "assets/brdf.dds"},                //
                     {"playerConfig", "assets/configs/player.json"}, //
                     {"sceneConfig", "assets/configs/scene.json"},   //
                     {"materialsPath", "assets/materials/"},         //
                     {"meshesPath", "assets/meshes/"},               //
                     {"backdropsPath", "assets/backdrops/"},
                     {"modelsPath", "assets/models/"}}; //

    json player = {{"height", 1.7},             //
                   {"mass", 100},               //
                   {"velocityMax", 6.0},        //
                   {"timeToReachVMax", 0.6},    //
                   {"timeToStopFromVMax", 0.1}, //
                   {"jumpHeight", 1.0}};        //

    json scene = {{"backdrop", "default"}, //
                  {"models", {}}};         //

    json backdrop = {{"color", "assets/backdrops/default/color.dds"},           //
                     {"radiance", "assets/backdrops/default/radiance.dds"},     //
                     {"irradiance", "assets/backdrops/default/irradiance.dds"}, //
                     {"light", {-10, -15, 0}},
                     {"brightness", 30}}; //

    json material = {{"diffuse", "diffuse.dds"},     //
                     {"normal", "normal.dds"},       //
                     {"metallic", "metallic.dds"},   //
                     {"roughness", "roughness.dds"}, //
                     {"ao", "ao.dds"},
                     {"scale", 1}}; //

    json mesh = {{"file", "default.glb"}, //
                 {"size", {2, 2, 2}}};    //

    json model = {{"mesh", "default"},     //
                  {"material", "default"}, //
                  {"mass", 0},             //
                  {"position", {0, 0, 0}}, //
                  {"rotation", {0, 0, 0}}, //
                  {"scale", {1, 1, 1}}};   //
};

}; // namespace tat
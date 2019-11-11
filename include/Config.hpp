#pragma once

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <spdlog/spdlog.h>

namespace tat
{

constexpr float PI = 3.1415926F;

class Config
{
  public:
    Config() = default;
    explicit Config(const std::string &path);
    ~Config() = default;

    std::string name = "none";
    float zNear = 0.1F;
    float zFar = 1024.F;
    int windowWidth = 1024;
    int windowHeight = 768;
    float FoV = 60.F;
    float mouseSensitivity = 25.F;
    bool sync = true;
    float shadowSize = 1024.F;
    std::string brdf = "assets/brdf.dds";
    std::string playerConfigPath = "assets/configs/player.json";
    std::string materialsConfigPath = "assets/configs/materials.json";
    std::string meshesConfigPath = "assets/configs/meshes.json";
    std::string backdropsConfigPath = "assets/configs/backdrops.json";
    std::string sceneConfigPath = "assets/configs/scene.json";

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
};

class PlayerConfig
{
  public:
    PlayerConfig() = default;
    explicit PlayerConfig(const std::string &path);
    ~PlayerConfig() = default;
    glm::vec3 position{};
    glm::vec3 rotation{};
    float height = 1.7F; // meters 1.0f == 1m
    float mass = 100.F;
    float jumpHeight = 1.F;
    float velocityMax = 6.F;
    float timeToReachVMax = 0.6F;
    float timeToStopfromVMax = 0.1F;

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
};

struct BackdropConfig
{
    std::string name = "desert";
    std::string colorPath = "assets/backdrop/desert/color.dds";
    std::string radiancePath = "assets/backdrop/desert/radiance.dds";
    std::string irradiancePath = "assets/backdrop/desert/irradiance.dds";
    glm::vec3 light = glm::vec3(0.F);
};

class BackdropsConfig
{
  public:
    BackdropsConfig() = default;
    explicit BackdropsConfig(const std::string &path);
    ~BackdropsConfig() = default;

    std::vector<BackdropConfig> backdrops{};

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
};

struct MaterialConfig
{
    std::string name = "default";
    std::string diffuse = "assets/materials/default/diffuse.dds";
    std::string normal = "assets/materials/default/normal.dds";
    std::string roughness = "assets/materials/default/roughness.dds";
    std::string metallic = "assets/materials/default/metallic.dds";
    std::string ao = "assets/materials/default/ao.dds";
};

class MaterialsConfig
{
  public:
    MaterialsConfig() = default;
    explicit MaterialsConfig(const std::string &path);
    ~MaterialsConfig() = default;

    std::vector<MaterialConfig> materials{};

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
};

struct MeshConfig
{
    std::string name = "cube";
    std::string path = "assets/models/cube.glb";
    glm::vec3 size{};
};

class MeshesConfig
{
  public:
    MeshesConfig() = default;
    explicit MeshesConfig(const std::string &path);
    ~MeshesConfig() = default;

    std::vector<MeshConfig> meshes{};

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
};

struct ModelConfig
{
    std::string name = "default";
    std::string mesh = "cube";
    std::string material = "default";
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};
    float mass = 0.F;
};

class SceneConfig
{
  public:
    SceneConfig() = default;
    explicit SceneConfig(const std::string &path);
    ~SceneConfig() = default;

    void createSceneConfig(const std::string &path);
    std::string name = "default";
    std::string backdrop = "desert";
    std::vector<ModelConfig> models{};

  private:
    std::shared_ptr<spdlog::logger> debugLogger;
};

} // namespace tat
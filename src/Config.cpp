#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "Config.hpp"

using json = nlohmann::json;

namespace tat
{

template <typename T> void load(const json &j, const std::string &type, const std::string &key, T &t)
{
    auto logger = spdlog::get("debugLogger");
    if (j.find(type) != j.end())
    {

        if (j.at(type).find(key) == j.at(type).end())
        {
            logger->warn("Config value in {} for {} not found", type, key);
        }
        else
        {
            t = j.at(type).at(key);
        }
    }
    else
    {
        logger->warn("Config {} not found", type);
    }
    logger->info("Config {} {}={} ", type, key, t);
}

Config::Config(const std::string &path)
{
    debugLogger = spdlog::get("debugLogger");
    if (std::filesystem::exists(path))
    { // if path exists load it
        try
        {
            debugLogger->info("Loading Config {}", path);
            std::ifstream file(path);
            json j;
            file >> j;

            load(j, "settings", "name", name);
            load(j, "settings", "zNear", zNear);
            load(j, "settings", "zFar", zFar);
            load(j, "settings", "windowWidth", windowWidth);
            load(j, "settings", "windowHeight", windowHeight);
            load(j, "settings", "mouseSensitivity", mouseSensitivity);
            load(j, "player", "FoV", FoV);
            load(j, "settings", "sync", sync);
            load(j, "settings", "shadowSize", shadowSize);
            load(j, "settings", "brdfPath", brdf);
            load(j, "settings", "playerConfigPath", playerConfigPath);
            load(j, "settings", "backdropsConfigPath", backdropsConfigPath);
            load(j, "settings", "materialsConfigPath", materialsConfigPath);
            load(j, "settings", "meshesConfigPath", meshesConfigPath);
            load(j, "settings", "sceneConfigPath", sceneConfigPath);
        }
        catch (json::exception &e)
        {
            debugLogger->error("Error {}", e.what());
        }
    }
    else
    { // just let default values be used
        debugLogger->warn("Unable to load {}", path);
    }
}

PlayerConfig::PlayerConfig(const std::string &path)
{
    debugLogger = spdlog::get("debugLogger");
    if (std::filesystem::exists(path))
    { // if path exists load it
        try
        {
            debugLogger->info("Loading Config {}", path);
            std::ifstream file(path);
            json j;
            file >> j;
            load(j, "player", "height", height);
            load(j, "player", "mass", mass);
            load(j, "player", "velocityMax", velocityMax);
            load(j, "player", "timeToReachVMax", timeToReachVMax);
            load(j, "player", "timeToStopFromVMax", timeToStopfromVMax);
            load(j, "player", "jumpHeight", jumpHeight);
        }
        catch (json::exception &e)
        {
            debugLogger->error("Error {}", e.what());
        }
    }
    else
    { // just let default values be used
        debugLogger->warn("Unable to load {}", path);
    }
}

BackdropsConfig::BackdropsConfig(const std::string &path)
{
    debugLogger = spdlog::get("debugLogger");
    if (std::filesystem::exists(path))
    { // if path exists load it
        try
        {
            debugLogger->info("Loading Config {}", path);
            std::ifstream file(path);
            json j;
            file >> j;
            if (j.find("backdrops") != j.end())
            {
                auto b = j.at("backdrops");
                for (auto &[key, backdrop] : b.items())
                {
                    BackdropConfig c{};
                    c.name = key;
                    load(b, key, "color", c.colorPath);
                    load(b, key, "radiance", c.radiancePath);
                    load(b, key, "irradiance", c.irradiancePath);

                    if (backdrop.find("light") != backdrop.end())
                    {
                        load(backdrop, "light", "x", c.light.x);
                        load(backdrop, "light", "y", c.light.y);
                        load(backdrop, "light", "z", c.light.z);
                    }

                    backdrops.push_back(c);
                }
            }
            else
            {
                debugLogger->warn("Unable to find backdrops in {}", path);
                backdrops.reserve(1);
            }
        }
        catch (json::exception &e)
        {
            debugLogger->error("Error {}", e.what());
            backdrops.resize(1);
        }
    }
    else
    { // otherwise resize to 1 using default backdrop
        backdrops.resize(1);
        debugLogger->warn("Unable to load {}", path);
    }
}

MaterialsConfig::MaterialsConfig(const std::string &path)
{
    debugLogger = spdlog::get("debugLogger");
    if (std::filesystem::exists(path))
    {
        try
        {
            debugLogger->info("Loading Config {}", path);
            std::ifstream file(path);
            json j;
            file >> j;

            if (j.find("materials") != j.end())
            {
                auto m = j.at("materials");
                for (auto &[key, material] : m.items())
                {
                    MaterialConfig c{};
                    c.name = key;
                    load(m, key, "diffuse", c.diffuse);
                    load(m, key, "normal", c.normal);
                    load(m, key, "metallic", c.metallic);
                    load(m, key, "roughness", c.roughness);
                    load(m, key, "ao", c.ao);
                    materials.push_back(c);
                }
            }
            else
            {
                debugLogger->warn("Unable to find materials in {}", path);
                materials.resize(1);
            }
        }
        catch (json::exception &e)
        {
            debugLogger->error("Error {}", e.what());
            materials.resize(1);
        }
    }
    else
    { // otherwise resize to 1 using default material
        materials.resize(1);
        debugLogger->warn("Unable to load {}", path);
    }
}

MeshesConfig::MeshesConfig(const std::string &path)
{
    debugLogger = spdlog::get("debugLogger");
    if (std::filesystem::exists(path))
    { // if meshes file exists load it
        try
        {
            debugLogger->info("Loading Config {}", path);
            std::ifstream file(path);
            json j;
            file >> j;

            if (j.find("meshes") != j.end())
            {
                auto m = j.at("meshes");
                for (auto &[key, mesh] : m.items())
                {
                    MeshConfig c{};
                    c.name = key;
                    load(m, key, "path", c.path);
                    if (mesh.find("size") != mesh.end())
                    {
                        load(mesh, "size", "x", c.size.x);
                        load(mesh, "size", "y", c.size.y);
                        load(mesh, "size", "z", c.size.z);
                    }
                    meshes.push_back(c);
                }
            }
            else
            {
                debugLogger->warn("Unable to find meshes in {}", path);
                meshes.resize(1);
            }
        }
        catch (json::exception &e)
        {
            debugLogger->error("Error {}", e.what());
            meshes.resize(1);
        }
    }
    else
    { // otherwise resize to 1 using default mesh
        meshes.resize(1);
        debugLogger->warn("Unable to load {}", path);
    }
}

SceneConfig::SceneConfig(const std::string &path)
{
    debugLogger = spdlog::get("debugLogger");
    if (std::filesystem::exists(path))
    { // if file exists load it
        try
        {
            debugLogger->info("Loading Config {}", path);
            std::ifstream file(path);
            json j;
            file >> j;

            if (j.find("scene") != j.end())
            {
                auto s = j.at("scene");
                name = s.value("name", name);
                backdrop = s.value("backdrop", backdrop);

                if (s.find("models") == s.end())
                {
                    models.resize(1);
                }
                else
                {
                    auto m = s.at("models");
                    for (auto &[key, model] : m.items())
                    {
                        ModelConfig c{};
                        c.name = key;
                        load(m, key, "mesh", c.mesh);
                        load(m, key, "material", c.material);
                        load(m, key, "mass", c.mass);
                        if (model.find("rotation") != model.end())
                        {
                            load(model, "rotation", "x", c.rotation.x);
                            load(model, "rotation", "y", c.rotation.y);
                            load(model, "rotation", "z", c.rotation.z);
                        }
                        if (model.find("scale") != model.end())
                        {
                            load(model, "scale", "x", c.scale.x);
                            load(model, "scale", "y", c.scale.y);
                            load(model, "scale", "z", c.scale.z);
                        }
                        if (model.find("position") != model.end())
                        {
                            load(model, "position", "x", c.position.x);
                            load(model, "position", "y", c.position.y);
                            load(model, "position", "z", c.position.z);
                        }

                        models.push_back(c);
                    }
                }
            }
            else
            {
                debugLogger->warn("Unable to find scene in {}", path);
            }
        }
        catch (json::exception &e)
        {
            debugLogger->error("Error {}", e.what());
            models.resize(1);
        }
    }
    else
    { // otherwise resize to 1 using default model
        models.resize(1);
        debugLogger->warn("Unable to load {} using default values.", path);
    }
}

} // namespace tat
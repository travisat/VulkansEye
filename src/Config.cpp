#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "Config.hpp"
#include "State.hpp"
#include "spdlog/spdlog.h"

using json = nlohmann::json;

namespace tat
{

Config::Config(const std::string &path)
{
    logger = spdlog::default_logger();

    try
    {
        // setup state with default vaules
        auto &state = State::instance();

        state["settings"] = settings;
        state["player"] = player;
        state["backdrops"]["default"] = backdrop;
        state["materials"]["default"] = material;
        state["meshes"]["default"] = mesh;
        state["models"]["default"] = model;
        state["scene"] = scene;
       
        // load settings
        loadSettings(path);
        loadPlayer(state["settings"]["playerConfig"]);
        loadScene(state["settings"]["sceneConfig"]);
        loadBackdrops(state["settings"]["backdropsPath"]);
        loadMaterials(state["settings"]["materialsPath"]);
        loadMeshes(state["settings"]["meshesPath"]);
        loadModels(state["settings"]["modelsPath"]);
        logger->info("Config Loaded"); 
    }
    catch (json::exception &e)
    {
        logger->error("Error {}", e.what());
    }
} 

void Config::loadSettings(const std::string &path)
{
    auto &state = State::instance();
    // load settings
    if (std::filesystem::exists(path))
    { // if path exists load it

        logger->info("Loading Config {}", path);
        std::ifstream file(path);
        json j;
        file >> j;

        state["settings"].update(j);
    }
    else
    { // just let default values be used
        logger->warn("Unable to load {}", path);
    }
}

void Config::loadPlayer(const std::string &path)
{
    auto &state = State::instance();

    if (std::filesystem::exists(path))
    {
        logger->info("Loading Config {}", path);
        std::ifstream file(path);
        json j;
        file >> j;

        state["player"].update(j);
    }
    else
    {
        logger->warn("Unable to load {}", path);
    }
}

void Config::loadScene(const std::string &path)
{
    auto &state = State::instance();
    if (std::filesystem::exists(path))
    {
        logger->info("Loading Config {}", path);
        std::ifstream file(path);
        json j;
        file >> j;

        state["scene"].update(j);
    }
    else
    {
        logger->warn("Unable to load {}", path);
    }
}

void Config::loadBackdrops(const std::string &path)
{
    auto &state = State::instance();

    if (std::filesystem::exists(path))
    {
        for (const auto &config : std::filesystem::directory_iterator(path))
        {
            auto configFile = config.path().string() + "/backdrop.json";
            if (std::filesystem::exists(configFile))
            {
                logger->info("Loading Config {}", configFile);

                std::ifstream file(configFile);
                json j;
                file >> j;

                // Don't just copy json in
                // Update existing correct values if available
                // extra entries don't matter*, missing ones do
                // TODO(travis) *they probably do matter
                auto name = config.path().filename().string();
                state["backdrops"][name] = backdrop;
                for (auto &item : j.items())
                {
                    state["backdrops"][name].update(item);
                }
            }
            else
            {
                logger->warn("Unable to load {}", configFile);
            }
        }
    }
    else
    {
        logger->warn("Unable to load backdrop path {}", path);
    }
}

void Config::loadMaterials(const std::string &path)
{
    auto &state = State::instance();

    if (std::filesystem::exists(path))
    {
        for (const auto &config : std::filesystem::directory_iterator(path))
        {
            auto configFile = config.path().string() + "/material.json";
            if (std::filesystem::exists(configFile))
            {
                logger->info("Loading Config {}", configFile);

                std::ifstream file(configFile);
                json j;
                file >> j;

                // Don't just copy json in
                // Update existing correct values if available
                // extra entries don't matter*, missing ones do
                // TODO(travis) *they probably do matter
                auto name = config.path().filename().string();
                state["materials"][name] = material;
                for (auto &item : j.items())
                {
                    state["materials"][name].update(item);
                }
            }
            else
            {
                logger->warn("Unable to load {}", configFile);
            }
        }
    }
    else
    {
        logger->warn("Unable to load material path {}", path);
    }
}

void Config::loadMeshes(const std::string &path)
{
    auto &state = State::instance();

    if (std::filesystem::exists(path))
    {
        for (const auto &config : std::filesystem::directory_iterator(path))
        {
            auto configFile = config.path().string() + "/mesh.json";
            if (std::filesystem::exists(configFile))
            {
                logger->info("Loading Config {}", configFile);

                std::ifstream file(configFile);
                json j;
                file >> j;

                // Don't just copy json in
                // Update existing correct values if available
                // extra entries don't matter*, missing ones do
                // TODO(travis) *they probably do matter
                auto name = config.path().filename().string();
                state["meshes"][name] = mesh;
                for (auto &item : j.items())
                {
                    state["meshes"][name].update(item);
                }
            }
            else
            {
                logger->warn("Unable to load {}", configFile);
            }
        }
    }
    else
    {
        logger->warn("Unable to load mesh path {}", path);
    }
}

void Config::loadModels(const std::string &path)
{
    auto &state = State::instance();

    if (std::filesystem::exists(path))
    {
        for (const auto &config : std::filesystem::directory_iterator(path))
        {
            auto configFile = config.path().string();
            if (std::filesystem::exists(configFile))
            {
                if (config.path().extension() == ".json")
                {
                    logger->info("Loading Config {}", configFile);

                    std::ifstream file(configFile);
                    json j;
                    file >> j;

                    // Don't just copy json in
                    // Update existing correct values if available
                    // extra entries don't matter*, missing ones do
                    // TODO(travis) *they probably do matter
                    auto name = config.path().filename().stem().string();
                    state["models"][name] = model;
                    for (auto &item : j.items())
                    {
                        state["models"][name].update(item);
                    }
                }
            }
            else
            {
                logger->warn("Unable to load {}", configFile);
            }
        }
    }
    else
    {
        logger->warn("Unable to load model path {}", path);
    }
}

} // namespace tat
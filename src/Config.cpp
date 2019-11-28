#include "Config.hpp"
#include "State.hpp"

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace tat
{

void Config::create(const std::string &path)
{
    try
    {
        spdlog::info("Loading Config");
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
        loadPlayer(state.at("settings").at("playerConfig"));
        loadScene(state.at("settings").at("sceneConfig"));
        loadBackdrops(state.at("settings").at("backdropsPath"));
        loadMaterials(state.at("settings").at("materialsPath"));
        loadMeshes(state.at("settings").at("meshesPath"));
        loadModels(state.at("settings").at("modelsPath"));

        if constexpr (Debug::enable)
        {
            spdlog::info("Created Config");
        }
    }
    catch (json::exception &e)
    {
        spdlog::error("Error {}", e.what());
    }
}

void Config::loadSettings(const std::string &path)
{
    auto &state = State::instance();
    // load settings
    if (std::filesystem::exists(path))
    { // if path exists load it
        std::ifstream file(path);
        json j;
        file >> j;

        state.at("settings").update(j);

        if constexpr (Debug::enable)
        {
            spdlog::info("Loaded Config {}", path);
        }
    }
    else
    { // just let default values be used
        spdlog::warn("Unable to load {}", path);
    }
}

void Config::loadPlayer(const std::string &path)
{
    auto &state = State::instance();

    if (std::filesystem::exists(path))
    {
        std::ifstream file(path);
        json j;
        file >> j;

        state.at("player").update(j);

        if constexpr (Debug::enable)
        {
            spdlog::info("Loaded Config {}", path);
        }
    }
    else
    {
        spdlog::warn("Unable to load {}", path);
    }
}

void Config::loadScene(const std::string &path)
{
    auto &state = State::instance();
    if (std::filesystem::exists(path))
    {
        std::ifstream file(path);
        json j;
        file >> j;

        state.at("scene").update(j);

        if constexpr (Debug::enable)
        {
            spdlog::info("Loaded Config {}", path);
        }
    }
    else
    {
        spdlog::warn("Unable to load {}", path);
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
                std::ifstream file(configFile);
                json j;
                file >> j;

                // Don't just copy json in
                // Update existing correct values if available
                // extra entries don't matter*, missing ones do
                // TODO(travis) *they probably do matter
                auto name = config.path().filename().string();
                state.at("backdrops")[name] = backdrop;
                for (auto &item : j.items())
                {
                    state.at("backdrops").at(name).update(item);
                }

                if constexpr (Debug::enable)
                {
                    spdlog::info("Loaded Config {}", configFile);
                }
            }
            else
            {
                spdlog::warn("Unable to load {}", configFile);
            }
        }
    }
    else
    {
        spdlog::warn("Unable to load backdrop path {}", path);
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
                std::ifstream file(configFile);
                json j;
                file >> j;

                // Don't just copy json in
                // Update existing correct values if available
                // extra entries don't matter*, missing ones do
                // TODO(travis) *they probably do matter
                auto name = config.path().filename().string();
                state.at("materials")[name] = material;
                for (auto &item : j.items())
                {
                    state.at("materials").at(name).update(item);
                }

                if constexpr (Debug::enable)
                {
                    spdlog::info("Loaded Config {}", configFile);
                }
            }
            else
            {
                spdlog::warn("Unable to load {}", configFile);
            }
        }
    }
    else
    {
        spdlog::warn("Unable to load material path {}", path);
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
                std::ifstream file(configFile);
                json j;
                file >> j;

                // Don't just copy json in
                // Update existing correct values if available
                // extra entries don't matter*, missing ones do
                // TODO(travis) *they probably do matter
                auto name = config.path().filename().string();
                state.at("meshes")[name] = mesh;
                for (auto &item : j.items())
                {
                    state.at("meshes").at(name).update(item);
                }

                if constexpr (Debug::enable)
                {
                    spdlog::info("Loaded Config {}", configFile);
                }
            }
            else
            {
                spdlog::warn("Unable to load {}", configFile);
            }
        }
    }
    else
    {
        spdlog::warn("Unable to load mesh path {}", path);
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
                    std::ifstream file(configFile);
                    json j;
                    file >> j;

                    // Don't just copy json in
                    // Update existing correct values if available
                    // extra entries don't matter*, missing ones do
                    // TODO(travis) *they probably do matter
                    auto name = config.path().filename().stem().string();
                    state.at("models")[name] = model;
                    for (auto &item : j.items())
                    {
                        state.at("models").at(name).update(item);
                    }

                    if constexpr (Debug::enable)
                    {
                        spdlog::info("Loaded Config {}", configFile);
                    }
                }
            }
            else
            {
                spdlog::warn("Unable to load {}", configFile);
            }
        }
    }
    else
    {
        spdlog::warn("Unable to load model path {}", path);
    }
}

} // namespace tat
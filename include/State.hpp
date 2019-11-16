#pragma once
#include <memory>
#include <utility>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Engine.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include "Overlay.hpp"
#include "Scene.hpp"
#include "Collection.hpp"

namespace tat
{

// State is stored in json, this is a singleton
// There cannot be more than one state.
// This setup is intended to make operating on state using scripts
// or another interface easy/possible
// ie being able to have a terminal like interface to move/create/update state
// Right now it holds initial state from config and some members are updated
// Additionally holds shared ptrs to the state classes that are single
class State: public json
{
  public:
    State(State const &) = delete;
    State(State&&) = delete;
    void operator=(State const &) = delete;
    void operator=(State&&) = delete;

    static auto instance() -> State &
    {
        static State instance;
        return instance;
    };
    
    std::shared_ptr<Engine> engine;
    std::shared_ptr<Window> window;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Player> player;
    std::shared_ptr<Overlay> overlay;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Collection<Backdrop>> backdrops;
    std::shared_ptr<Collection<Material>> materials;
    std::shared_ptr<Collection<Mesh>> meshes;
    std::shared_ptr<Collection<Model>> models;

  private:
    State() = default;
};

} // namespace tat
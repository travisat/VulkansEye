#pragma once
#include <memory>
#include <utility>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "engine/Engine.hpp"
#include "engine/Window.hpp"

#include "overlay/Overlay.hpp"

#include "Camera.hpp"
#include "Collection.hpp"
#include "Player.hpp"
#include "Scene.hpp"


namespace tat
{



// State is stored in json, this is a singleton
// There cannot be more than one state.
// This setup is intended to make operating on state using scripts
// or another interface easy/possible
// ie being able to have a terminal like interface to move/create/update state
// Right now it holds initial state from config and some members are updated
// Additionally holds shared ptrs to the state classes that are single
class State : public json
{
  public:
    State(State const &) = delete;
    State(State &&) = delete;
    void operator=(State const &) = delete;
    void operator=(State &&) = delete;

    static auto instance() -> State &
    {
        static State instance;
        return instance;
    };

    Engine engine{};
    Window window{};
    Camera camera{};
    Player player{};
    Overlay overlay{};
    Scene scene{};
    Collection<Backdrop> backdrops{};
    Collection<Material> materials{};
    Collection<Mesh> meshes{};
    Collection<Model> models{};

  private:
    State() = default;
};

} // namespace tat
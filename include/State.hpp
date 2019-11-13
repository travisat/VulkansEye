#pragma once
#include <memory>
#include <utility>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace tat
{

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

  private:
    State() = default;
};

} // namespace tat
#pragma once

#include "engine/Window.hpp"

namespace tat
{

class Paused
{
  public:
    void create();
    void show();

  private:
    void close();
    Window *window = nullptr;
};

} // namespace tat
#pragma once
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <chrono>
#include <ctime>
#include <iostream>

namespace tat
{

class Timer
{
  public:
    Timer(Timer const &) = delete;
    void operator=(Timer const &) = delete;

    static auto getInstance() -> Timer &
    {
        static Timer instance;
        return instance;
    };

    // returns high resolution time in seconds since first call of function
    static auto time() -> float
    {
        static std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    };

    static auto systemTime() -> float
    {
        static auto startTime = std::chrono::system_clock::now();
        auto currentTime = std::chrono::system_clock::now();
        return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    };

  private:
    Timer()= default;;
};

} // namespace tat
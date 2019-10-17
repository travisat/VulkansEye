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

    static Timer &getInstance()
    {
        static Timer instance;
        return instance;
    };

    // returns milliseconts since first call of function
    static float getCount()
    {
        return getInstance().getCountIMPL();
    };

    static std::chrono::steady_clock::time_point getTime()
    {
        return getInstance().getTimeIMPL();
    };

    static float systemTime()
    {
        return getInstance().systemTimeIMPL();
    };

  private:
    Timer(void){};

    float getCountIMPL()
    {
        static std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - startTime).count();
    };

    std::chrono::steady_clock::time_point getTimeIMPL()
    {
        return std::chrono::high_resolution_clock::now();
    };

    float systemTimeIMPL()
    {
        static auto startTime = std::chrono::system_clock::now();
        auto currentTime = std::chrono::system_clock::now();
        return std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    }
};

} // namespace tat
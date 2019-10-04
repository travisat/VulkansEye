#pragma once
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <iostream>
#include <chrono>
#include <ctime>

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

    //returns milliseconts since first call of function
    static double getCount()
    {
        return getInstance().getCountIMPL();
    };

    static std::chrono::steady_clock::time_point getTime()
    {
        return getInstance().getTimeIMPL();
    };

    static double systemTime()
    {
       return getInstance().systemTimeIMPL();
    };

private:
    Timer(void){};

    double getCountIMPL()
    {
        static std::chrono::steady_clock::time_point startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::chrono::milliseconds::period>(currentTime - startTime).count();
    };

    std::chrono::steady_clock::time_point getTimeIMPL()
    {
        return std::chrono::high_resolution_clock::now();
    };

    double systemTimeIMPL()
    {
        static auto startTime = std::chrono::system_clock::now();
        auto currentTime = std::chrono::system_clock::now();
        return std::chrono::duration<double, std::chrono::seconds::period>(currentTime - startTime).count();
    }
};
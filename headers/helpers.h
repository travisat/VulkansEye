#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <cassert>
#include <iostream>
#include <sstream>
#include <vulkan/vulkan.h>

// These need to be templated functions

namespace tat
{

template <typename... Args> void Trace(Args &&... args)
{
    std::ostringstream stream;
    (stream << ... << std::forward<Args>(args)) << '\n';

    OutputDebugString(stream.str().c_str());
}

template <typename T> auto CheckResult(T result) -> T
{
    if (result != 0)
    {
        std::ostringstream stream;
        stream << " Error result is " << result << " in " << __FILE__ << " at line " << __LINE__ << std::endl;
        OutputDebugString(stream.str().c_str());
        assert(result == 0);
    }
    return result;
}

} // namespace tat
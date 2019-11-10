#pragma once

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <vulkan/vulkan.hpp>

#include <glm/ext.hpp>

// These need to be templated functions

namespace tat
{

inline auto readFile(const std::string &filename) -> std::vector<char>
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
};

/*template <typename... Args> void Trace(Args &&... args)
{
    std::ostringstream stream;
    (stream << ... << std::forward<Args>(args)) << '\n';
    std::clog << stream.str();
}*/

inline auto CheckResult(vk::Result result) -> vk::Result
{
    if (result != vk::Result::eSuccess)
    {
        std::ostringstream stream;
        stream << " Error result is " << result << " in " << __FILE__ << " at line " << __LINE__ << std::endl;
        std::cerr << stream.str();
        assert(result == vk::Result::eSuccess);
    }
    return result;
}

template <typename T> auto CheckResult(T result) -> T
{
    if (result != 0)
    {
        std::ostringstream stream;
        stream << " Error result is " << result << " in " << __FILE__ << " at line " << __LINE__ << std::endl;
        std::cerr << stream.str();
        assert(result == 0);
    }
    return result;
}

} // namespace tat
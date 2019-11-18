#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{
class PipelineCache
{
  public:
    void create();
    void destroy();

    vk::PipelineCache pipelineCache = nullptr;

  private:
};
} // namespace tat
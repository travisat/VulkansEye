#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{
class Semaphore
{
  public:
  Semaphore();
  ~Semaphore();

  vk::Semaphore semaphore = nullptr;

  private:
};
} // namespace tat
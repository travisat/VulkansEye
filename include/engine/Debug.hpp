#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <vk_mem_alloc.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

class Debug
{
  public:
    void create();
    void destroy();

    vk::DebugUtilsMessengerEXT debugMessenger = nullptr;
    //TODO(travis) make #ndebug work with this, something is overriding it
    const bool enableValidationLayers = true;
    const std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

  private:
};

} // namespace tat
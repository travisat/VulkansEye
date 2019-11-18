#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

template <typename T>
auto getHandle(T const &cppHandle) -> uint64_t {
  return uint64_t(static_cast<typename T::CType>(cppHandle));
};

class Debug
{
  public:
    void create();
    void destroy();

    static void setMarker(uint64_t object, vk::ObjectType type, const std::string &name);

    vk::DebugUtilsMessengerEXT debugMessenger = nullptr;
    // TODO(travis) make #ndebug work with this, something is overriding it
    const bool enableValidationLayers = true;
    const std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

  private:
};

} // namespace tat
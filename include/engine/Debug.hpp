#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace tat
{

class Debug
{
  public:
    void create(vk::Instance *instance);
    void destroy();

    template <typename T>
    static void setName(const vk::Device &device, const T &t, const std::string_view &name)
    {
        vk::DebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.objectHandle = uint64_t(static_cast<typename T::CType>(t));;
        nameInfo.objectType = t.objectType;
        nameInfo.pObjectName = name.data();
        device.setDebugUtilsObjectNameEXT(nameInfo);
    };

    vk::DebugUtilsMessengerEXT debugMessenger = nullptr;
    // TODO(travis) make #ndebug work with this, something is overriding it
    
    static constexpr bool  enableValidationLayers = true;
    const std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

  private:
    vk::Instance *instance = nullptr;
};

} // namespace tat
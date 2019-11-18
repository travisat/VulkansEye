#include "engine/Debug.hpp"
#include "State.hpp"

#include <spdlog/spdlog.h>

namespace tat
{

VKAPI_ATTR auto VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *
                                         /*pUserData*/) -> VkBool32
{
    std::string prefix;

    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
    {
        spdlog::warn("Validation {}", pCallbackData->pMessage);
        std::cerr << pCallbackData->pMessage << std::endl;
    }
    else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
    {
        spdlog::error("Validation {}", pCallbackData->pMessage);
        std::cerr << pCallbackData->pMessage << std::endl;
    }
    else
    {
        spdlog::info("Validation {}", pCallbackData->pMessage);
        std::cout << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

void Debug::create()
{
    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    debugInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    debugInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    debugInfo.pfnUserCallback = debugCallback;

    auto &instance = State::instance().engine.instance;
    debugMessenger = instance.createDebugUtilsMessengerEXT(debugInfo);
}

void Debug::destroy()
{
    if (debugMessenger)
    {
        auto &instance = State::instance().engine.instance;
        instance.destroyDebugUtilsMessengerEXT(debugMessenger);
    }
}

} // namespace tat
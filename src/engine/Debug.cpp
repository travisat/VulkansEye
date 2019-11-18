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

    auto logger = spdlog::get("validation");

    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
    {
        logger->warn("Validation {}", pCallbackData->pMessage);
        std::cerr << pCallbackData->pMessage << std::endl;
    }
    else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
    {
        logger->error("Validation {}", pCallbackData->pMessage);
        std::cerr << pCallbackData->pMessage << std::endl;
    }
    else
    {
        logger->info("Validation {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}

void Debug::create()
{
    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    debugInfo.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
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

void Debug::setMarker(uint64_t object, vk::ObjectType type, const std::string &name)
{
    vk::DebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.objectHandle = object;
    nameInfo.objectType = type;
    nameInfo.pObjectName = name.c_str();

    auto &device = State::instance().engine.device;
    device.setDebugUtilsObjectNameEXT(nameInfo);
}

} // namespace tat
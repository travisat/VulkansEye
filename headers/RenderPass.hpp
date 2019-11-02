#pragma once

#include <array>

#include "Vulkan.hpp"
#include "vulkan/vulkan.hpp"

namespace tat
{

auto createColorPass(Vulkan *vulkan) -> vk::RenderPass;
auto createShadowPass(Vulkan *vulkan) -> vk::RenderPass;

} // namespace tat
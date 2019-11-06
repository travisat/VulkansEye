#pragma once

#include <array>

#include "Vulkan.hpp"

namespace tat
{

auto createColorPass(Vulkan *vulkan) -> vk::RenderPass;
auto createShadowPass(Vulkan *vulkan) -> vk::RenderPass;

} // namespace tat
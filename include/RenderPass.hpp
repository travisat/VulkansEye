#pragma once

#include <vulkan/vulkan.hpp>

namespace tat
{

auto createColorPass() -> vk::RenderPass;
auto createShadowPass() -> vk::RenderPass;

} // namespace tat
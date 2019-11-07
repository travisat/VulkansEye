#pragma once

#include <array>
#include <memory>

#include "Vulkan.hpp"

namespace tat
{

auto createColorPass(const std::shared_ptr<Vulkan> &vulkan) -> vk::RenderPass;
auto createShadowPass(const std::shared_ptr<Vulkan> &vulkan) -> vk::RenderPass;

} // namespace tat
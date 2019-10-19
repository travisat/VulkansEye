#pragma once

#include <array>

#include "Vulkan.hpp"

namespace tat
{

void createRenderPass(VkRenderPass &renderPass, Vulkan *vulkan);
void createOffscreenRenderPass(VkRenderPass &renderPass, Vulkan *vulkan);


} // namespace tat
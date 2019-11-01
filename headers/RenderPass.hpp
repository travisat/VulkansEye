#pragma once

#include <array>

#include "Vulkan.hpp"

namespace tat
{

void createColorPass(Vulkan *vulkan);
void createShadowPass(Vulkan *vulkan);
void createSunPass(Vulkan *vulkan);


} // namespace tat
#include "engine/PipelineCache.hpp"
#include "State.hpp"

namespace tat
{

void PipelineCache::create()
{
    auto &device = State::instance().engine.device;
    vk::PipelineCacheCreateInfo createInfo{};
    pipelineCache = device.createPipelineCache(createInfo);
}

void PipelineCache::destroy()
{
    if (pipelineCache)
    {
        auto &device = State::instance().engine.device;
        device.destroyPipelineCache(pipelineCache);
    }
}

} // namespace tat
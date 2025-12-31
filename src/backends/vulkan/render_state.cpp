#include "render_state.hpp"

#include <backends/vulkan/vulkan_engine.hpp>

RenderState::RenderState() {}

RenderState::RenderState(Engine* engine, VkDevice device, VmaAllocator allocator) : engine(engine), device(device), allocator(allocator) {}

void RenderState::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {
    engine->immediate_submit(std::move(function));
}

#include "render_state.hpp"

#include <backends/vulkan/vulkan_engine.hpp>

RenderState::RenderState() {}

RenderState::RenderState(Engine* engine, VkDevice device, uint32_t queue_family_index, VmaAllocator allocator, VkExtent2D draw_extent, VkFormat draw_image_format, VkFormat depth_image_format)
    : engine(engine), device(device), queue_family_index(queue_family_index), allocator(allocator), draw_extent(draw_extent), draw_image_format(draw_image_format), depth_image_format(depth_image_format) {}

void RenderState::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {
    engine->immediate_submit(std::move(function));
}

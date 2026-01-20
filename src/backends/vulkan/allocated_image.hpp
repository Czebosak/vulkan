#pragma once
#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <backends/vulkan/render_state.hpp>

class AllocatedImage {
public:
    VkImage image;
    VkImageView image_view;
    VmaAllocation allocation;
    VkExtent3D image_extent;
    VkFormat image_format;

    [[nodiscard]] static AllocatedImage create(RenderState& render_state, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false, VkImageType type = VK_IMAGE_TYPE_2D);
    [[nodiscard]] static AllocatedImage create(RenderState& render_state, void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false, VkImageType type = VK_IMAGE_TYPE_2D);

    void destroy(RenderState& render_state);
};

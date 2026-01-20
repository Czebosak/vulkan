#include "allocated_image.hpp"
#include "backends/vulkan/resource_loader.hpp"

#include <backends/vulkan/initializers.hpp>
#include <backends/vulkan/defines.hpp>
#include <backends/vulkan/allocated_buffer.hpp>
#include <backends/vulkan/images.hpp>

#include <cmath>
#include <vulkan/vulkan_core.h>

AllocatedImage AllocatedImage::create(RenderState& render_state, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped, VkImageType type) {
    AllocatedImage new_image = {
        .image_extent = size,
        .image_format = format,
    };

    VkImageCreateInfo img_info = vkinit::image_create_info(format, usage, size);
    if (mipmapped) {
        img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
    }

    // always allocate images on dedicated GPU memory
    VmaAllocationCreateInfo alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    // allocate and create the image
    VK_CHECK(vmaCreateImage(render_state.allocator, &img_info, &alloc_info, &new_image.image, &new_image.allocation, nullptr));

    // if the format is a depth format, we will need to have it use the correct
    // aspect flag
    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format == VK_FORMAT_D32_SFLOAT) {
        aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    // build a image-view for the image
    VkImageViewCreateInfo view_info = vkinit::imageview_create_info(format, new_image.image, aspectFlag);
    view_info.subresourceRange.levelCount = img_info.mipLevels;

    VK_CHECK(vkCreateImageView(render_state.device, &view_info, nullptr, &new_image.image_view));

    return new_image;
}

AllocatedImage AllocatedImage::create(RenderState& render_state, void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped, VkImageType type) {
    size_t data_size = size.depth * size.width * size.height * 4;
    AllocatedBuffer staging_buffer = AllocatedBuffer::create(render_state, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    memcpy(staging_buffer.info.pMappedData, data, data_size);

    AllocatedImage new_image = AllocatedImage::create(render_state, size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

    render_state.resource_loader->add_job(resource::Job {
        .func = [&](VkCommandBuffer cmd) {
            vkutil::transition_image(cmd, new_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkBufferImageCopy copy_region = {
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
                .imageExtent = size,
            };

            // copy the buffer into the image
            vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

            vkutil::transition_image(cmd, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            return true;
        }
    });

    staging_buffer.destroy(render_state.allocator);

    return new_image;
}

void AllocatedImage::destroy(RenderState& render_state) {
    vkDestroyImageView(render_state.device, image_view, nullptr);
    vmaDestroyImage(render_state.allocator, image, allocation);
}

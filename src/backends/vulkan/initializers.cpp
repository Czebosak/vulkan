#include "initializers.hpp"
#include <vulkan/vulkan_core.h>

namespace vkinit {
    VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags) {
        VkFenceCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
        };

        return info;
    }

    VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags) {
        VkSemaphoreCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
        };

        return info;
    }
    
    VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags, VkCommandBufferInheritanceInfo* inheritance_info_ptr) {
        VkCommandBufferBeginInfo info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = flags,
            .pInheritanceInfo = inheritance_info_ptr,
        };

        return info;
    }

    VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspect_mask) {
        VkImageSubresourceRange sub_image {
            .aspectMask = aspect_mask,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        };

        return sub_image;
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count) {
        VkCommandBufferAllocateInfo info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,

            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = count,
        };

        return info;
    }

    VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore) {
        VkSemaphoreSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = semaphore,
            .value = 1,
            .stageMask = stage_mask,
            .deviceIndex = 0,
        };

        return submit_info;
    }

    VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd) {
        VkCommandBufferSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = cmd,
            .deviceMask = 0,
        };

        return submit_info;
    }

    VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signal_semaphore_info, VkSemaphoreSubmitInfo* wait_semaphore_info) {
        VkSubmitInfo2 submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .waitSemaphoreInfoCount = uint32_t(wait_semaphore_info == nullptr) ? uint32_t(0) : uint32_t(1),
            .pWaitSemaphoreInfos = wait_semaphore_info,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = cmd,
            .signalSemaphoreInfoCount = uint32_t(signal_semaphore_info == nullptr) ? uint32_t(0) : uint32_t(1),
            .pSignalSemaphoreInfos = signal_semaphore_info,
        };

        return submit_info;
    }

    VkRenderingAttachmentInfo attachment_info(VkImageView view, VkClearValue* clear, VkImageLayout layout) {
        VkRenderingAttachmentInfo color_attachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = view,
            .imageLayout = layout,
            .loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };

        if (clear) {
            color_attachment.clearValue = *clear;
        }

        return color_attachment;
    }


    VkRenderingAttachmentInfo depth_attachment_info(VkImageView view, VkImageLayout layout) {
        VkRenderingAttachmentInfo depth_attachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = view,
            .imageLayout = layout,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };

        depth_attachment.clearValue.depthStencil.depth = 0.0f;

        return depth_attachment;
    }

    VkRenderingInfo rendering_info(VkExtent2D render_extent, VkRenderingAttachmentInfo* color_attachment, VkRenderingAttachmentInfo* depth_attachment) {
        VkRenderingInfo render_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,

            .renderArea = VkRect2D { VkOffset2D { 0, 0 }, render_extent },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = color_attachment,
            .pDepthAttachment = depth_attachment,
            .pStencilAttachment = nullptr,
        };

        return render_info;
    }

    VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent, VkImageType type) {
        VkImageCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = extent,
            .mipLevels = 1,
            .arrayLayers = 1,
            //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
            .samples = VK_SAMPLE_COUNT_1_BIT,
            //optimal tiling, which means the image is stored on the best gpu format
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage_flags,
        };

        return info;
    }

    VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags) {
        // build a image-view for the depth image to use for rendering
        VkImageViewCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange = {
                .aspectMask = aspect_flags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };

        return info;
    }
}

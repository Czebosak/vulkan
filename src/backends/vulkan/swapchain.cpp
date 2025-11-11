#include "swapchain.hpp"

#include <limits>
#include <algorithm>

namespace hayvk {
    std::expected<Swapchain, VkResult> Swapchain::create(VkDevice device, VkSwapchainKHR handle, VkExtent2D extent, VkSurfaceFormatKHR surface_format, VkPresentModeKHR present_mode) {
        Swapchain swapchain;
        swapchain.handle = handle;
        swapchain.extent = extent;
        swapchain.surface_format = surface_format;
        swapchain.present_mode = present_mode;
        
        uint32_t image_count;
        vkGetSwapchainImagesKHR(device, handle, &image_count, nullptr);

        std::vector<VkImage> image_handles(image_count);
        vkGetSwapchainImagesKHR(device, handle, &image_count, image_handles.data());

        swapchain.images.resize(image_count);

        VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
        };

        for (size_t i = 0; i < image_count; i++) {
            VkImageViewCreateInfo color_image_view = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = surface_format.format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1
                },
            };

            swapchain.images[i].image = image_handles[i];
            color_image_view.image = image_handles[i];
            
            VkResult result = vkCreateImageView(device, &color_image_view, nullptr, &swapchain.images[i].view);
            if (result != VK_SUCCESS) return std::unexpected(result);

            result = vkCreateSemaphore(device, &semaphore_create_info, NULL, &swapchain.images[i].draw_complete_semaphore);
            if (result != VK_SUCCESS) return std::unexpected(result);

            bool separate_present_queue = true;
            if (separate_present_queue) {
                result = vkCreateSemaphore(device, &semaphore_create_info, NULL, &swapchain.images[i].image_ownership_semaphore);
                if (result != VK_SUCCESS) return std::unexpected(result);
            }
        }

        return std::move(swapchain);
    }

    void Swapchain::destroy(VkDevice device) {
        for (auto& image : images) {
            vkDestroyImageView(device, image.view, nullptr);
            vkDestroyImage(device, image.image, nullptr);
        }

        vkDestroySwapchainKHR(device, handle, nullptr);
    }

    SwapchainBuilder::SwapChainSupportDetails::SwapChainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

        if (format_count != 0) {
            formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

        if (present_mode_count != 0) {
            present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
        }
    }

    VkSurfaceFormatKHR SwapchainBuilder::SwapChainSupportDetails::choose_surface_format(VkSurfaceFormatKHR preferred_format) {
        for (const auto& format : formats) {
            if (format.format == preferred_format.format && format.colorSpace == preferred_format.colorSpace) {
                return format;
            }
        }

        return formats[0];
    }

    constexpr VkPresentModeKHR fallback_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    VkPresentModeKHR SwapchainBuilder::SwapChainSupportDetails::choose_present_mode(VkPresentModeKHR preferred_present_mode) {
        VkPresentModeKHR chosen_present_mode = fallback_present_mode;

        if (chosen_present_mode != fallback_present_mode) {
            for (const auto& present_mode : present_modes) {
                if (present_mode == preferred_present_mode) {
                    chosen_present_mode = present_mode;
                    break;
                }
            }
        }

        return chosen_present_mode;
    }

    VkExtent2D SwapchainBuilder::SwapChainSupportDetails::choose_swap_extent(GLFWwindow* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actual_extent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actual_extent;
        }
    }

    std::expected<Swapchain, VkResult> SwapchainBuilder::build(
        VkDevice device,
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface,
        GLFWwindow* window,
        VkPresentModeKHR preferred_present_mode,
        VkSurfaceFormatKHR preferred_surface_format
    ) {
        SwapChainSupportDetails details(physical_device, surface);

        VkSurfaceFormatKHR surface_format = details.choose_surface_format(preferred_surface_format);
        VkPresentModeKHR present_mode = details.choose_present_mode(preferred_present_mode);
        VkExtent2D extent = details.choose_swap_extent(window);

        uint32_t image_count = details.capabilities.minImageCount + 1;
        
        if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
            image_count = details.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchain_create_info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .minImageCount = image_count,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = extent,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // Temporary
            .presentMode = present_mode,
            .clipped = VK_TRUE,
        };

        VkSwapchainKHR swapchain_handle;
        VkResult result = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain_handle);
        if (result != VK_SUCCESS) return std::unexpected(result);

        return std::move(Swapchain::create(device, swapchain_handle, extent, surface_format, present_mode));
    }
}

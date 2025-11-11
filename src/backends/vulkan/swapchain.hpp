#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <expected>

namespace hayvk {
    class Swapchain {
    public:
        struct SwapchainImage {
            VkImage image;
            VkImageView view;
            VkFramebuffer framebuffer;
            VkSemaphore draw_complete_semaphore;
            VkSemaphore image_ownership_semaphore;
        };

        VkSwapchainKHR handle;
        VkExtent2D extent;
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;

        std::vector<SwapchainImage> images;

        static std::expected<Swapchain, VkResult> create(VkDevice device, VkSwapchainKHR handle, VkExtent2D extent, VkSurfaceFormatKHR surface_format, VkPresentModeKHR present_mode);

        void destroy(VkDevice device);
    };

    class SwapchainBuilder {
    private:
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;

            SwapChainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

            VkSurfaceFormatKHR choose_surface_format(VkSurfaceFormatKHR preferred_format);
            VkPresentModeKHR choose_present_mode(VkPresentModeKHR preferred_present_mode);
            VkExtent2D choose_swap_extent(GLFWwindow* window);
        };
    public:
        std::expected<Swapchain, VkResult> build(
            VkDevice device,
            VkPhysicalDevice physical_device,
            VkSurfaceKHR surface,
            GLFWwindow* window,
            VkPresentModeKHR preferred_present_mode = VK_PRESENT_MODE_FIFO_KHR,
            VkSurfaceFormatKHR preferred_surface_format = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
        );
    };
}
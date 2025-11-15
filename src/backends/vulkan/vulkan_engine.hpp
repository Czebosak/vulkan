#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include <vector>

#include <backends/vulkan/types.hpp>
#include <backends/vulkan/descriptors.hpp>

constexpr uint32_t WIDTH = 640*2;
constexpr uint32_t HEIGHT = 480*2;

constexpr bool ENABLE_VALIDATION_LAYERS = true;

class Engine {
private:
    GLFWwindow* window;

	VkDebugUtilsMessengerEXT debug_messenger;

    VkInstance instance;

    VkPhysicalDevice physical_device;
    VkDevice device;

    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain;
	VkFormat swapchain_image_format;

	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	VkExtent2D swapchain_extent;

    DeletionQueue deletion_queue;

	VmaAllocator allocator;

	AllocatedImage draw_image;
	VkExtent2D draw_extent;

	DescriptorAllocator global_descriptor_allocator;

	VkDescriptorSet draw_image_descriptors;
	VkDescriptorSetLayout draw_image_descriptor_layout;

	static constexpr uint FRAME_OVERLAP = 2;

	FrameData frames[FRAME_OVERLAP];

	VkQueue graphics_queue;
	uint32_t graphics_queue_family;

	uint frame_number = 0;

	constexpr FrameData& get_current_frame() { return frames[frame_number % FRAME_OVERLAP]; };

	VkPipeline gradient_pipeline;
	VkPipelineLayout gradient_pipeline_layout;

    uint32_t init_vulkan(GLFWwindow* window);
	void init_swapchain();
	void init_commands();
	void init_sync_structures();
	void init_descriptors();
	void init_background_pipelines();
	void init_pipelines();

    void create_swapchain(uint32_t width, uint32_t height);
	void destroy_swapchain();

	void draw_background(VkCommandBuffer cmd);
public:
	uint32_t init();

	void clean_up();

	void draw();

	void run();
};

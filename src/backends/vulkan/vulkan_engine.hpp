#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include <glm/glm.hpp>

#include <vector>

#include <backends/vulkan/types.hpp>
#include <backends/vulkan/descriptors.hpp>

constexpr uint32_t WIDTH = 640*2;
constexpr uint32_t HEIGHT = 480*2;

constexpr bool ENABLE_VALIDATION_LAYERS = true;

struct ComputePushConstants {
	glm::vec4 data1;
	glm::vec4 data2;
	glm::vec4 data3;
	glm::vec4 data4;
};

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

	/* std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views; */
	std::vector<SwapchainImage> swapchain_images;
	VkExtent2D swapchain_extent;

    DeletionQueue deletion_queue;

	VmaAllocator allocator;

	AllocatedImage draw_image;
	VkExtent2D draw_extent;

	DescriptorAllocator global_descriptor_allocator;

	VkDescriptorSet draw_image_descriptors;
	VkDescriptorSetLayout draw_image_descriptor_layout;

	static constexpr uint FRAME_OVERLAP = 2;

	uint frame_number = 0;

	inline FrameData& get_current_frame() { return frames[frame_number % FRAME_OVERLAP]; };

	FrameData frames[FRAME_OVERLAP];

	VkQueue graphics_queue;
	uint32_t graphics_queue_family;

	VkPipeline gradient_pipeline;
	VkPipelineLayout gradient_pipeline_layout;

	ComputePushConstants push_constants;

    VkFence imm_fence;
    VkCommandBuffer imm_command_buffer;
    VkCommandPool imm_command_pool;

    uint32_t init_vulkan(GLFWwindow* window);
	void init_swapchain();
	void init_commands();
	void init_sync_structures();
	void init_descriptors();
	void init_background_pipelines();
	void init_pipelines();
	void init_imgui();

    void create_swapchain(uint32_t width, uint32_t height);
	void destroy_swapchain();

	void draw_imgui(VkCommandBuffer cmd, VkImageView target_image_view);
	void draw_background(VkCommandBuffer cmd);

	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
public:
	uint32_t init();

	void clean_up();

	void draw();

	void run();
};

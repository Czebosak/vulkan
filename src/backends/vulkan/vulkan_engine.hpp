#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

#include <glm/glm.hpp>

#include <vector>

#include <backends/vulkan/types.hpp>
#include <backends/vulkan/descriptors.hpp>
#include <backends/vulkan/allocated_buffer.hpp>

#include <voxel/mesh_generation.hpp>

#include <input.hpp>
#include <game_state.hpp>

// constexpr uint32_t WIDTH = 640*2;
// constexpr uint32_t HEIGHT = 480*2;

constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;

constexpr bool ENABLE_VALIDATION_LAYERS = true;

struct ComputePushConstants {
    glm::vec4 data1;
    glm::vec4 data2;
    glm::vec4 data3;
    glm::vec4 data4;
};

class RenderState;

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

    VkPipelineLayout triangle_pipeline_layout;
    VkPipeline triangle_pipeline;

    VkPipelineLayout mesh_pipeline_layout;
    VkPipeline mesh_pipeline;

    VkPipelineLayout voxel_pipeline_layout;
    VkPipeline voxel_pipeline;

    GPUMeshBuffers rectangle;

    voxel::Mesh chunk_mesh;

    input::Input input;

    GameState game_state;
    
    RenderState render_state;

    friend RenderState;

    AllocatedBuffer create_buffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
    void destroy_buffer(const AllocatedBuffer& buffer);

    uint32_t init_vulkan(GLFWwindow* window);
    void init_swapchain();
    void init_commands();
    void init_sync_structures();
    void init_descriptors();
    void init_background_pipelines();
    void init_pipelines();
    void init_imgui();
    void init_triangle_pipeline();
    void init_mesh_pipeline();
    void init_voxel_pipeline();
    void init_default_data();

    void create_swapchain(uint32_t width, uint32_t height);
    void destroy_swapchain();

    void draw_imgui(VkCommandBuffer cmd, VkImageView target_image_view);
    void draw_background(VkCommandBuffer cmd);
    void draw_geometry(VkCommandBuffer cmd);
    void draw_chunk(VkCommandBuffer cmd, const voxel::Chunk& chunk);

    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
public:
    uint32_t init();

    void clean_up();

    void draw();

    void run();
};

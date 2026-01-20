#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <vk_mem_alloc.h>

#include <deque>
#include <functional>

#include <glm/glm.hpp>

struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function);
    void flush();
};

struct FrameData {
    VkCommandPool command_pool;
    VkCommandBuffer main_command_buffer;

    VkSemaphore swapchain_semaphore, render_semaphore, acquire_semaphore;
    VkFence render_fence;

    DeletionQueue deletion_queue;
};

struct SwapchainImage {
    VkImage handle;
    VkImageView view;
    VkSemaphore submit_semaphore;
};

struct Vertex {
    glm::vec3 position;
    float _pad0;
    glm::vec3 normal;
    float _pad1;
    glm::vec2 uv;
    glm::vec2 _pad2;
};

// push constants for our mesh object draws
struct GPUDrawPushConstants {
    glm::mat4 world_matrix;
    VkDeviceAddress vertex_buffer;
};

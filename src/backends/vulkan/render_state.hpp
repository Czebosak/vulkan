#pragma once
#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <functional>

#include <backends/vulkan/resource_loader.hpp>

class Engine;

class RenderState {
private:
    Engine* engine;

    RenderState();

    RenderState(
        Engine* engine,
        VkDevice device,
        uint32_t queue_family_index,
        VmaAllocator allocator,
        VkExtent2D draw_extent,
        VkFormat draw_image_format,
        VkFormat depth_image_format,

        resource::ResourceLoader& resource_loader
    );
    
    friend Engine;
public:
    VkDevice device;
    uint32_t queue_family_index;
    VmaAllocator allocator;
    VkExtent2D draw_extent;
    VkFormat draw_image_format;
    VkFormat depth_image_format;

    resource::ResourceLoader* resource_loader;

    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
};

#pragma once
#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <functional>

class Engine;

class RenderState {
private:
    Engine* engine;
public:
    VkDevice device;
    VmaAllocator allocator;

    RenderState();

    RenderState(Engine* engine, VkDevice device, VmaAllocator allocator);

    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
};

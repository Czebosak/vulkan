#include "allocated_buffer.hpp"

AllocatedBuffer AllocatedBuffer::create(RenderState& render_state, size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage) {
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = alloc_size,
        .usage = usage,
    };

    VmaAllocationCreateInfo vma_alloc_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = memory_usage,
    };

    AllocatedBuffer new_buffer;
    VK_CHECK(vmaCreateBuffer(render_state.allocator, &buffer_info, &vma_alloc_info, &new_buffer.buffer, &new_buffer.allocation, &new_buffer.info));

    return new_buffer;
}

void AllocatedBuffer::destroy(VmaAllocator allocator) {
    vmaDestroyBuffer(allocator, buffer, allocation);
}

/* template <typename VERTEX_TYPE, typename INDEX_TYPE>
GPUMeshBuffers upload_mesh(RenderState render_state, std::span<VERTEX_TYPE> vertices, std::span<INDEX_TYPE> indices) {
    const size_t vertex_buffer_size = vertices.size() * sizeof(VERTEX_TYPE);
    const size_t index_buffer_size = indices.size() * sizeof(INDEX_TYPE);

    GPUMeshBuffers new_surface;

    new_surface.vertex_buffer = AllocatedBuffer::create(render_state, vertex_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo device_adress_info{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = new_surface.vertex_buffer.buffer };
    new_surface.vertex_buffer_address = vkGetBufferDeviceAddress(render_state.device, &device_adress_info);

    new_surface.index_buffer = AllocatedBuffer::create(render_state, index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = AllocatedBuffer::create(render_state, vertex_buffer_size + index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = staging.info.pMappedData;

    memcpy(data, vertices.data(), vertex_buffer_size);
    memcpy((char*)data + vertex_buffer_size, indices.data(), index_buffer_size);

    immediate_submit([&](VkCommandBuffer cmd) {
        VkBufferCopy vertexCopy = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = vertex_buffer_size,
        };

        vkCmdCopyBuffer(cmd, staging.buffer, new_surface.vertex_buffer.buffer, 1, &vertexCopy);

        VkBufferCopy indexCopy = {
            .srcOffset = vertex_buffer_size,
            .dstOffset = 0,
            .size = index_buffer_size,
        };

        vkCmdCopyBuffer(cmd, staging.buffer, new_surface.index_buffer.buffer, 1, &indexCopy);
    });

    staging.destroy(render_state.allocator);

    return new_surface;
} */

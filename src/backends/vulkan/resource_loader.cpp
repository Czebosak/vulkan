#include "resource_loader.hpp"

#include <bitset>

#include <backends/vulkan/vulkan_function_pointers.hpp>
#include <backends/vulkan/defines.hpp>
#include <backends/vulkan/initializers.hpp>

constexpr size_t MAX_JOB_COUNT = 50;

void resource::ResourceLoader::run(std::stop_token st) {
    while (!st.stop_requested()) {
        Job job[MAX_JOB_COUNT];
        size_t job_count = jobs.wait_dequeue_bulk(job, MAX_JOB_COUNT);
        std::bitset<MAX_JOB_COUNT> jobs_executed;

        fmt::println("yes");

        VK_CHECK(vkResetCommandBuffer(command_buffer, 0));

        VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VK_CHECK(vkBeginCommandBuffer(command_buffer, &cmd_begin_info));

        for (int i = 0; i < job_count; i++) {
            jobs_executed[i] = job[i].func(command_buffer);
        }

        VK_CHECK(vkEndCommandBuffer(command_buffer));

        VK_CHECK(vkResetFences(device, 1, &fence));

        VkCommandBufferSubmitInfo submit_info = vkinit::command_buffer_submit_info(command_buffer);
        VkSubmitInfo2 submit = vkinit::submit_info(&submit_info, nullptr, nullptr);
        //VK_CHECK(vkQueueSubmit2(transfer_queue, 1, &submit, fence));
        vkQueueSubmit2(transfer_queue, 1, &submit, fence);

        fmt::println("WEEE ARE");
        vkWaitForFences(device, 1, &fence, true, 1000000000 /* 1s */);
        //VK_CHECK(vkWaitForFences(device, 1, &fence, true, 1000000000 /* 1s */));
        fmt::println("HEERE");


        for (int i = 0; i < job_count; i++) {
            if (jobs_executed[i]) {
                job[i].callback();
            }
        }
    }
}

void resource::ResourceLoader::init(const vkb::Device& vkb_device) {
    transfer_queue = vkb_device.get_queue(vkb::QueueType::transfer).value();
    transfer_queue_family = vkb_device.get_queue_index(vkb::QueueType::transfer).value();

    device = vkb_device.device;

    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = transfer_queue_family,
    };

    VK_CHECK(vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool));

    VkCommandBufferAllocateInfo cmd_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VK_CHECK(vkAllocateCommandBuffers(device, &cmd_alloc_info, &command_buffer));

    VkFenceCreateInfo fence_create_info = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VK_CHECK(vkCreateFence(device, &fence_create_info, nullptr, &fence));

    #ifndef NDEBUG
    VkDebugUtilsObjectNameInfoEXT name_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL,
        .objectHandle = (uint64_t)fence,
        .pObjectName = "Resource loader fence",
    };
    pVkSetDebugUtilsObjectNameEXT(device, &name_info);
    #endif

    thread = std::jthread(&ResourceLoader::run, this);
}

void resource::ResourceLoader::add_job(Job&& job) {
    jobs.enqueue(std::forward<Job>(job));
}

void resource::ResourceLoader::destroy() {
    vkDestroyCommandPool(device, command_pool, nullptr);
    thread.request_stop();
}

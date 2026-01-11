#pragma once

#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

#include <thread>
#include <functional>

#include <blockingconcurrentqueue.h>

namespace resource {
    struct Job {
        std::move_only_function<bool(VkCommandBuffer)> func;
        std::move_only_function<void()> callback;
    };

    class ResourceLoader {
    private:
        std::jthread thread;

        VkDevice device;

        uint32_t transfer_queue_family;
        VkQueue transfer_queue;

        VkFence fence;
        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;

        moodycamel::BlockingConcurrentQueue<Job> jobs;

        void run(std::stop_token st);
    public:
        void init(const vkb::Device& vkb_device);

        void add_job(Job&& job);

        void destroy();
    };
}

/* #if defined(_WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
    #define VK_USE_PLATFORM_X11_KHR   // or VK_USE_PLATFORM_WAYLAND_KHR
    #define GLFW_EXPOSE_NATIVE_X11    // or GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h> */

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>

/* #include <backends/vulkan/builders.hpp>
#include <backends/vulkan/pipeline_builder.hpp>
#include <backends/vulkan/swapchain.hpp>
#include <backends/vulkan/types.hpp> */

//const int MAX_FRAMES_IN_FLIGHT = 2;

//const uint WIDTH = 800;
//const uint HEIGHT = 600;

//VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger) {
//    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
//    if (func != nullptr) {
//        return func(instance, create_info, allocator, debug_messenger);
//    } else {
//        return VK_ERROR_EXTENSION_NOT_PRESENT;
//    }
//}
//
//void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator) {
//    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//    if (func != nullptr) {
//        func(instance, debug_messenger, allocator);
//    }
//}
//
//struct QueueFamilyIndices {
//    std::optional<uint32_t> graphics_family;
//    std::optional<uint32_t> present_family;
//
//    bool is_complete() {
//        return graphics_family.has_value() && present_family.has_value();
//    }
//};
//
//struct SwapChainSupportDetails {
//    VkSurfaceCapabilitiesKHR capabilities;
//    std::vector<VkSurfaceFormatKHR> formats;
//    std::vector<VkPresentModeKHR> present_modes;
//};
//
//class HelloTriangleApplication {
//public:
//    void run() {
//        init_window();
//        init_vulkan();
//        main_loop();
//        clean_up();
//    }
//private:
//    GLFWwindow* window;
//
//    VkDebugUtilsMessengerEXT debug_messenger;
//
//    VkInstance instance;
//
//    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
//    VkDevice device;
//
//    VkSurfaceKHR surface;
//
//    VkQueue graphics_queue;
//    VkQueue present_queue;
//
//    /* VkSwapchainKHR swap_chain;
//    VkFormat swap_chain_image_format;
//    VkExtent2D swap_chain_extent;
//    std::vector<VkImage> swap_chain_images;
//    std::vector<VkImageView> swap_chain_image_views;
//    std::vector<VkFramebuffer> swap_chain_framebuffers; */
//    hayvk::Swapchain swapchain;
//
//    AllocatedImage draw_image;
//
//    VmaAllocator allocator;
//
//    VkRenderPass render_pass;
//    VkPipelineLayout pipeline_layout;
//    VkPipeline graphics_pipeline;
//
//    VkCommandPool command_pool;
//    std::vector<VkCommandBuffer> command_buffers;
//
//    std::vector<VkSemaphore> image_available_semaphores;
//    std::vector<VkSemaphore> render_finished_semaphores;
//    std::vector<VkFence> in_flight_fences;
//
//    uint32_t current_frame = 0;
//
//    const std::vector<const char*> validation_layers = {
//        "VK_LAYER_KHRONOS_validation"
//    };
//
//    const std::vector<const char*> device_extensions = {
//        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
//    };
//
//    #ifdef NDEBUG
//        const bool enable_validation_layers = false;
//    #else
//        const bool enable_validation_layers = true;
//    #endif
//
//    void init_window() {
//        glfwInit();
//
//        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//
//        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan x3", nullptr, nullptr);
//    }
//
//    void init_vulkan() {
//        create_instance();
//        setup_debug_messenger();
//        create_surface();
//        pick_physical_device();
//        create_logical_device();
//
//        VmaAllocatorCreateInfo allocator_info = {
//            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
//            .physicalDevice = physical_device,
//            .device = device,
//            .instance = instance,
//        };
//
//	    vmaCreateAllocator(&allocator_info, &allocator);
//
//        create_swap_chain();
//        //create_image_views();
//        //create_render_pass();
//        create_graphics_pipeline();
//        //create_framebuffers();
//        create_command_pool();
//        create_command_buffers();
//        create_sync_objects();
//    }
//
//    void setup_debug_messenger() {
//        if (!enable_validation_layers) return;
//
//        VkDebugUtilsMessengerCreateInfoEXT create_info;
//        populate_debug_messenger_create_info(create_info);
//
//        if (create_debug_utils_messenger_ext(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
//            throw std::runtime_error("failed to set up debug messenger!");
//        }
//    }
//
//    void main_loop() {
//        while (!glfwWindowShouldClose(window)) {
//            glfwPollEvents();
//            draw_frame();
//        }
//
//        vkDeviceWaitIdle(device);
//    }
//
//    void clean_up() {
//        /* for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//            vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
//            vkDestroyFence(device, in_flight_fences[i], nullptr);
//        }
//
//        for (size_t i = 0; i < swap_chain_images.size(); i++) {
//            vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
//        } */
//
//        vkDestroyCommandPool(device, command_pool, nullptr);
//        
//        vkDestroyPipeline(device, graphics_pipeline, nullptr);
//        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
//        vkDestroyRenderPass(device, render_pass, nullptr);
//
//        swapchain.destroy(device);
//        vkDestroySurfaceKHR(instance, surface, nullptr);
//
//        vkDestroyDevice(device, nullptr);
//
//        if (enable_validation_layers) {
//            destroy_debug_utils_messenger_ext(instance, debug_messenger, nullptr);
//        }
//
//        vkDestroyInstance(instance, nullptr);
//
//        glfwDestroyWindow(window);
//
//        glfwTerminate();
//    }
//
//    void draw_frame() {
//        vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000);
//        vkResetFences(_device, 1, &get_current_frame()._renderFence);
//
//        uint32_t image_index;
//        vkAcquireNextImageKHR(device, swapchain.handle, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
//
//        vkResetCommandBuffer(command_buffers[current_frame],  0);
//        record_command_buffer(command_buffers[current_frame], image_index);
//
//        VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
//        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//
//        VkSemaphore signal_semaphores[] = {render_finished_semaphores[image_index]};
//
//        VkSubmitInfo submit_info {
//            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
//            .waitSemaphoreCount = 1,
//            .pWaitSemaphores = wait_semaphores,
//            .pWaitDstStageMask = wait_stages,
//            .commandBufferCount = 1,
//            .pCommandBuffers = &command_buffers[current_frame],
//            .signalSemaphoreCount = 1,
//            .pSignalSemaphores = signal_semaphores,
//        };
//
//        if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS) {
//            throw std::runtime_error("failed to submit draw command buffer!");
//        }
//
//        VkSwapchainKHR swap_chains[] = {swapchain.handle};
//
//        VkPresentInfoKHR present_info {
//            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
//            .waitSemaphoreCount = 1,
//            .pWaitSemaphores = signal_semaphores,
//            .swapchainCount = 1,
//            .pSwapchains = swap_chains,
//            .pImageIndices = &image_index,
//            .pResults = nullptr, // Optional
//        };
//
//        vkQueuePresentKHR(present_queue, &present_info);
//
//        current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
//    }
//
//    void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
//        VkCommandBufferBeginInfo begin_info {
//            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
//            .flags = 0, // Optional
//            .pInheritanceInfo = nullptr, // Optional
//        };
//
//        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
//            throw std::runtime_error("failed to begin recording command buffer!");
//        }
//
//        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
//
//        VkRenderingAttachmentInfo color_attachment {
//            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
//            .pNext = nullptr,
//            .imageView = swapchain.images[image_index].view,
//            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
//            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//        };
//
//        /* VkRenderingAttachmentInfo depth_attachment {
//            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
//            .pNext = nullptr,
//            .imageView = swap_chain_image_views[image_index],
//            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
//            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
//            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//        }; */
//
//        VkRenderingInfo render_info {
//            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
//            .pNext = nullptr,
//            .renderArea = VkRect2D { VkOffset2D { 0, 0 }, swapchain.extent },
//            .layerCount = 1,
//            .colorAttachmentCount = 1,
//            .pColorAttachments = &color_attachment,
//            .pDepthAttachment = nullptr,
//            .pStencilAttachment = nullptr,
//        };
//
//        vkCmdBeginRendering(command_buffer, &render_info);
//        {
//            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
//
//            VkViewport viewport {
//                .x = 0,
//                .y = 0,
//                .width = float(swapchain.extent.width),
//                .height = float(swapchain.extent.height),
//                .minDepth = 0.0f,
//                .maxDepth = 1.0f,
//            };
//
//            vkCmdSetViewport(command_buffer, 0, 1, &viewport);
//
//            VkRect2D scissor = {
//                .offset = {0, 0},
//                .extent = swapchain.extent,
//            };
//
//            vkCmdSetScissor(command_buffer, 0, 1, &scissor);
//
//            vkCmdDraw(command_buffer, 3, 1, 0, 0);
//        }
//        vkCmdEndRendering(command_buffer);
//
//        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
//            throw std::runtime_error("failed to record command buffer!");
//        }
//    }
//
//    void create_instance() {
//        if (enable_validation_layers && !check_validation_layer_support()) {
//            throw std::runtime_error("validation layers requested, but not available!");
//        }
//
//        VkApplicationInfo app_info;
//        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//        app_info.pApplicationName = "Hello Triangle";
//        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//        app_info.pEngineName = "No Engine";
//        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//        app_info.apiVersion = VK_API_VERSION_1_3;
//
//        VkInstanceCreateInfo create_info;
//        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//        create_info.pApplicationInfo = &app_info;
//
//        std::vector<const char*> extensions = get_required_extensions();
//        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
//        create_info.ppEnabledExtensionNames = extensions.data();
//
//        VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
//        if (enable_validation_layers) {
//            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
//            create_info.ppEnabledLayerNames = validation_layers.data();
//
//            populate_debug_messenger_create_info(debug_create_info);
//            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
//        } else {
//            create_info.enabledLayerCount = 0;
//
//            create_info.pNext = nullptr;
//        }
//
//        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create instance!");
//        }
//    }
//
//    bool check_validation_layer_support() {
//        uint32_t layer_count;
//        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
//
//        std::vector<VkLayerProperties> available_layers(layer_count);
//        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
//
//        for (const char* layer_name : validation_layers) {
//            bool layer_found = false;
//
//            for (const auto& layer_properties : available_layers) {
//                if (std::strcmp(layer_name, layer_properties.layerName) == 0) {
//                    layer_found = true;
//                    break;
//                }
//            }
//
//            if (!layer_found) {
//                return false;
//            }
//        }
//
//        return true;
//    }
//
//    std::vector<const char*> get_required_extensions() {
//        uint32_t glfw_extension_count = 0;
//        const char** glfw_extensions;
//        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
//
//        std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
//
//        if (enable_validation_layers) {
//            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//        }
//
//        return extensions;
//    }
//
//    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
//        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
//        VkDebugUtilsMessageTypeFlagsEXT message_type,
//        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
//        void* user_data
//    ) {
//        std::cerr << "validation layer: " << callback_data->pMessage << std::endl;
//
//        return VK_FALSE;
//    }
//
//    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
//        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//        create_info.pfnUserCallback = debug_callback;
//        create_info.pUserData = nullptr;
//        create_info.flags = 0;
//    }
//
//    void pick_physical_device() {
//        uint32_t device_count = 0;
//        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
//
//        if (device_count == 0) {
//            throw std::runtime_error("failed to find GPUs with Vulkan support!");
//        }
//
//        std::vector<VkPhysicalDevice> devices(device_count);
//        vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
//
//        for (const auto& device : devices) {
//            if (is_device_suitable(device)) {
//                physical_device = device;
//                break;
//            }
//        }
//
//        if (physical_device == VK_NULL_HANDLE) {
//            throw std::runtime_error("failed to find a suitable GPU!");
//        }
//    }
//
//    void create_logical_device() {
//        QueueFamilyIndices indices = find_queue_families(physical_device);
//
//        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
//        std::set<uint32_t> unique_queue_families = {
//            indices.graphics_family.value(),
//            indices.present_family.value()
//        };
//
//        float queuePriority = 1.0f;
//        for (uint32_t queue_family : unique_queue_families) {
//            VkDeviceQueueCreateInfo queue_create_info = {};
//            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//            queue_create_info.queueFamilyIndex = queue_family;
//            queue_create_info.queueCount = 1;
//            queue_create_info.pQueuePriorities = &queuePriority;
//            queue_create_infos.push_back(queue_create_info);
//        }
//
//        // VkPhysicalDeviceFeatures device_features = {};
//
//        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {
//            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
//            .dynamicRendering = VK_TRUE,
//        };
//
//        VkPhysicalDeviceFeatures2 features2 = {
//            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
//            .pNext = &dynamicRenderingFeatures,
//        };
//
//        VkDeviceCreateInfo create_info = {};
//        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//        create_info.pNext = &features2;
//
//        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
//        create_info.pQueueCreateInfos = queue_create_infos.data();
//
//        //create_info.pEnabledFeatures = &device_features;
//
//        create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
//        create_info.ppEnabledExtensionNames = device_extensions.data();
//
//        if (enable_validation_layers) {
//            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
//            create_info.ppEnabledLayerNames = validation_layers.data();
//        } else {
//            create_info.enabledLayerCount = 0;
//        }
//
//        if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create logical device!");
//        }
//
//        vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
//        vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
//    }
//
//    void create_surface() {
//        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create window surface!");
//        }
//    }
//
//    void create_swap_chain() {
//        auto result = hayvk::SwapchainBuilder().build(device, physical_device, surface, window); // VK_PRESENT_MODE_MAILBOX_KHR
//        if (!result)
//            throw std::runtime_error("swapchain failed to cerate");
//
//        swapchain = std::move(result.value());
//
//        /* create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//        create_info.subresourceRange.baseMipLevel = 0;
//        create_info.subresourceRange.levelCount = 1;
//        create_info.subresourceRange.baseArrayLayer = 0;
//        create_info.subresourceRange.layerCount = 1; */
//        /* SwapChainSupportDetails swap_chain_support = query_swap_chain_support(physical_device);
//
//        VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
//        VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
//        VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);
//        
//        uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
//        
//        if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
//            image_count = swap_chain_support.capabilities.maxImageCount;
//        }
//
//        VkSwapchainCreateInfoKHR create_info;
//        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//        create_info.surface = surface;
//
//        create_info.minImageCount = image_count;
//        create_info.imageFormat = surface_format.format;
//        create_info.imageColorSpace = surface_format.colorSpace;
//        create_info.imageExtent = extent;
//        create_info.imageArrayLayers = 1;
//        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//
//        QueueFamilyIndices indices = find_queue_families(physical_device);
//        uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};
//
//        if (indices.graphics_family != indices.present_family) {
//            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//            create_info.queueFamilyIndexCount = 2;
//            create_info.pQueueFamilyIndices = queue_family_indices;
//        } else {
//            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//            create_info.queueFamilyIndexCount = 0;
//            create_info.pQueueFamilyIndices = nullptr;
//        }
//
//        create_info.preTransform = swap_chain_support.capabilities.currentTransform;
//        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//        create_info.presentMode = present_mode;
//        create_info.clipped = VK_TRUE;
//        create_info.oldSwapchain = VK_NULL_HANDLE;
//
//        if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swap_chain) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create swap chain!");
//        }
//
//        vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
//        swap_chain_images.resize(image_count);
//        vkGetSwapchainImagesKHR(device, swap_chain, &image_count, swap_chain_images.data());
//
//        swap_chain_image_format = surface_format.format;
//        swap_chain_extent = extent;
//    }
//
//    void create_image_views() {
//        swap_chain_image_views.resize(swap_chain_images.size());
//
//        for (size_t i = 0; i < swap_chain_images.size(); i++) {
//            VkImageViewCreateInfo create_info = {};
//            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//            create_info.image = swap_chain_images[i];
//            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//            create_info.format = swap_chain_image_format;
//
//            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//
//            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//            create_info.subresourceRange.baseMipLevel = 0;
//            create_info.subresourceRange.levelCount = 1;
//            create_info.subresourceRange.baseArrayLayer = 0;
//            create_info.subresourceRange.layerCount = 1;
//
//            if (vkCreateImageView(device, &create_info, nullptr, &swap_chain_image_views[i]) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create image views!");
//            }
//        } */
//    }
//
//    void create_graphics_pipeline() {
//        auto vert_shader_module_opt = hayvk::builders::load_shader_module(device, "assets/vert.spv");
//        vk::UniqueShaderModule vert_shader_module = vert_shader_module_opt.has_value() ? std::move(*vert_shader_module_opt) : throw std::runtime_error("Couldn't construct vertex shader module");
//
//        auto frag_shader_module_opt = hayvk::builders::load_shader_module(device, "assets/frag.spv");
//        vk::UniqueShaderModule frag_shader_module = frag_shader_module_opt.has_value() ? std::move(*frag_shader_module_opt) : throw std::runtime_error("Couldn't construct vertex shader module");
//
//        auto pipeline_opt = hayvk::builders::PipelineBuilder {
//            .input_assembly = vk::PipelineInputAssemblyStateCreateInfo()
//                .setTopology(vk::PrimitiveTopology::eTriangleList)
//                .setPrimitiveRestartEnable(vk::False),
//            .rasterizer = vk::PipelineRasterizationStateCreateInfo()
//                .setDepthClampEnable(vk::False)
//                .setRasterizerDiscardEnable(vk::False)
//                .setPolygonMode(vk::PolygonMode::eFill)
//                .setCullMode(vk::CullModeFlagBits::eBack)
//                .setFrontFace(vk::FrontFace::eCounterClockwise)
//                .setDepthBiasEnable(VK_FALSE)
//                .setDepthBiasConstantFactor(0.0f)
//                .setDepthBiasClamp(0.0f)
//                .setDepthBiasSlopeFactor(0.0f)
//                .setLineWidth(1.0f),
//            .color_blend_attachment = vk::PipelineColorBlendAttachmentState()
//                .setBlendEnable(vk::True)
//                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
//                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
//                .setColorBlendOp(vk::BlendOp::eAdd)
//                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
//                .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
//                .setAlphaBlendOp(vk::BlendOp::eAdd)
//                .setColorWriteMask(vk::ColorComponentFlagBits::eR
//                                    | vk::ColorComponentFlagBits::eG
//                                    | vk::ColorComponentFlagBits::eB
//                                    | vk::ColorComponentFlagBits::eA),
//            //.pipeline_layout_create_info = vk::PipelineLayoutCreateInfo(),
//            .color_attachment_format = vk::Format(swapchain.surface_format.format),
//        }.set_shaders(*vert_shader_module, *frag_shader_module).build(device);
//
//        if (pipeline_opt.has_value()) {
//            graphics_pipeline = *pipeline_opt;
//        } else {
//            throw std::runtime_error("skibidi tiolet graphics pipepline got gyatted");
//        }
//
//        /* VkPipelineShaderStageCreateInfo vert_shader_stage_info {
//            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//            .stage = VK_SHADER_STAGE_VERTEX_BIT,
//            .module = vert_shader_module,
//            .pName = "main",
//        };
//
//        VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
//            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
//            .module = frag_shader_module,
//            .pName = "main",
//        };
//
//        VkPipelineShaderStageCreateInfo shader_stages[] {vert_shader_stage_info, frag_shader_stage_info};
//
//        VkPipelineVertexInputStateCreateInfo vertex_input_info {
//            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//            .vertexBindingDescriptionCount = 0,
//            .pVertexBindingDescriptions = nullptr,
//            .vertexAttributeDescriptionCount = 0,
//            .pVertexAttributeDescriptions = nullptr,
//        };
//
//        VkViewport viewport {
//            .x = 0.0f,
//            .y = 0.0f,
//            .width = (float) swap_chain_extent.width,
//            .height = (float) swap_chain_extent.height,
//            .minDepth = 0.0f,
//            .maxDepth = 1.0f,
//        };
//
//        VkRect2D scissor {
//            .offset = {0, 0},
//            .extent = swap_chain_extent,
//        };
//
//        VkPipelineViewportStateCreateInfo viewport_state {
//            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
//            .viewportCount = 1,
//            .pViewports = &viewport,
//            .scissorCount = 1,
//            .pScissors = &scissor,
//        };
//
//        VkPipelineColorBlendStateCreateInfo color_blending {
//            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
//            .logicOpEnable = VK_FALSE,
//            .logicOp = VK_LOGIC_OP_COPY, // Optional
//            .attachmentCount = 1,
//            .pAttachments = &color_blend_attachment,
//            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f} // Optional
//        };
//
//        VkPipelineLayoutCreateInfo pipeline_layout_info {
//            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
//            .setLayoutCount = 0, // Optional
//            .pSetLayouts = nullptr, // Optional
//            .pushConstantRangeCount = 0, // Optional
//            .pPushConstantRanges = nullptr, // Optional
//        };
//
//        if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create pipeline layout!");
//        }
//
//        VkGraphicsPipelineCreateInfo pipelineInfo {
//            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
//            .stageCount = 2,
//            .pStages = shader_stages,
//            .pVertexInputState = &vertex_input_info,
//            .pInputAssemblyState = &input_assembly,
//            .pViewportState = &viewport_state,
//            .pRasterizationState = &rasterizer,
//            .pMultisampleState = &multisampling,
//            .pDepthStencilState = nullptr, // Optional
//            .pColorBlendState = &color_blending,
//            .pDynamicState = nullptr,
//            .layout = pipeline_layout,
//            .renderPass = render_pass,
//            .subpass = 0,
//            .basePipelineHandle = VK_NULL_HANDLE, // Optional
//            .basePipelineIndex = -1, // Optional
//        };
//
//        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create graphics pipeline!");
//        }
//
//        vkDestroyShaderModule(device, frag_shader_module, nullptr);
//        vkDestroyShaderModule(device, vert_shader_module, nullptr); */
//    }
//
//    void create_render_pass() {
//        VkAttachmentDescription color_attachment {
//            .format = swapchain.surface_format.format,
//            .samples = VK_SAMPLE_COUNT_1_BIT,
//            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
//        };
//
//        VkAttachmentReference color_attachment_ref {
//            .attachment = 0,
//            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//        };
//
//        VkSubpassDescription subpass {
//            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//            .colorAttachmentCount = 1,
//            .pColorAttachments = &color_attachment_ref,
//        };
//        
//        VkSubpassDependency dependency {
//            .srcSubpass = VK_SUBPASS_EXTERNAL,
//            .dstSubpass = 0,
//            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
//            .srcAccessMask = 0,
//            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
//        };
//        
//        VkRenderPassCreateInfo render_pass_info {
//            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
//            .attachmentCount = 1,
//            .pAttachments = &color_attachment,
//            .subpassCount = 1,
//            .pSubpasses = &subpass,
//            .dependencyCount = 1,
//            .pDependencies = &dependency,
//        };
//
//        if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create render pass!");
//        }
//    }
//
//    /* void create_framebuffers() {
//        swap_chain_framebuffers.resize(swap_chain_image_views.size());
//
//        for (size_t i = 0; i < swap_chain_image_views.size(); i++) {
//            VkImageView attachments[] = {
//                swap_chain_image_views[i]
//            };
//
//            VkFramebufferCreateInfo framebufferInfo {
//                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//                .renderPass = render_pass,
//                .attachmentCount = 1,
//                .pAttachments = attachments,
//                .width = swap_chain_extent.width,
//                .height = swap_chain_extent.height,
//                .layers = 1,
//            };
//
//            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swap_chain_framebuffers[i]) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create framebuffer!");
//            }
//        }
//    } */
//
//    void create_command_pool() {
//        QueueFamilyIndices queue_family_indices = find_queue_families(physical_device);
//
//        VkCommandPoolCreateInfo pool_info {
//            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
//            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
//            .queueFamilyIndex = queue_family_indices.graphics_family.value(),
//        };
//
//        if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create command pool!");
//        }
//    }
//
//    void create_command_buffers() {
//        command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
//
//        VkCommandBufferAllocateInfo alloc_info {
//            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
//            .commandPool = command_pool,
//            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
//            .commandBufferCount = (uint32_t)command_buffers.size(),
//        };
//
//        if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
//            throw std::runtime_error("failed to allocate command buffers!");
//        }
//    }
//
//    void create_sync_objects() {
//        image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
//        //render_finished_semaphores.resize(swap_chain_images.size());
//        in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
//
//        VkSemaphoreCreateInfo semaphore_info {
//            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
//        };
//
//        VkFenceCreateInfo fence_info {
//            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
//            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
//        };
//        
//        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//            if (vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS ||
//                vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
//
//                throw std::runtime_error("failed to create synchronization objects for a frame!");
//            }
//        }
//
//        for (size_t i = 0; i < swapchain.images.size(); i++) {
//            if (vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS) {
//                throw std::runtime_error("failed to create synchronization objects for a frame!");
//            }
//        }
//    }
//
//    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
//        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
//            return capabilities.currentExtent;
//        } else {
//            int width, height;
//            glfwGetFramebufferSize(window, &width, &height);
//
//            VkExtent2D actual_extent = {
//                static_cast<uint32_t>(width),
//                static_cast<uint32_t>(height)
//            };
//
//            actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
//            actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
//
//            return actual_extent;
//        }
//    }
//
//    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
//        for (const auto& available_format : available_formats) {
//            if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//                return available_format;
//            }
//        }
//
//        return available_formats[0];
//    }
//
//    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
//        for (const auto& available_present_mode : available_present_modes) {
//            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
//                return available_present_mode;
//            }
//        }
//
//        return VK_PRESENT_MODE_FIFO_KHR;
//    }
//
//    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device) {
//        SwapChainSupportDetails details;
//
//        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
//
//        uint32_t format_count;
//        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
//
//        if (format_count != 0) {
//            details.formats.resize(format_count);
//            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
//        }
//
//        uint32_t present_mode_count;
//        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
//
//        if (present_mode_count != 0) {
//            details.present_modes.resize(present_mode_count);
//            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
//        }
//
//        return details;
//    }
//
//    bool is_device_suitable(VkPhysicalDevice device) {
//        QueueFamilyIndices indices = find_queue_families(device);
//
//        bool extensions_supported = check_device_extension_support(device);
//
//        bool swap_chain_adequate = false;
//        if (extensions_supported) {
//            SwapChainSupportDetails swap_chain_support = query_swap_chain_support(device);
//            swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
//        }
//
//        return indices.is_complete() && extensions_supported && swap_chain_adequate;
//    }
//
//    bool check_device_extension_support(VkPhysicalDevice device) {
//        uint32_t extension_count;
//        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
//
//        std::vector<VkExtensionProperties> available_extensions(extension_count);
//        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());
//
//        std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
//
//        for (const VkExtensionProperties& extension : available_extensions) {
//            required_extensions.erase(extension.extensionName);
//        }
//
//        return required_extensions.empty();
//    }
//
//    QueueFamilyIndices find_queue_families(VkPhysicalDevice device) {
//        QueueFamilyIndices indices;
//
//        uint32_t queue_family_count = 0;
//        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
//
//        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
//        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
//
//        int i = 0;
//        for (const auto& queueFamily : queue_families) {
//            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//                indices.graphics_family = i;
//            }
//
//            VkBool32 present_support = false;
//            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
//
//            if (present_support) {
//                indices.present_family = i;
//            }
//
//            if (indices.is_complete()) {
//                break;
//            }
//
//            i++;
//        }
//
//        return indices;
//    }
//};

#include <engine.hpp>

#include <backends/vulkan/voxel_renderer.hpp>

int main() {
    Engine engine;

    uint32_t result = engine.init();

    if (!result) {
        engine.run();

        engine.clean_up();
    } else {
        #ifdef USE_OPENGL
        std::cout << "OpenGL engine initialization failed with error code: " << result << std::endl;
        #else
        std::cout << "Vulkan engine initialization failed with error code: " << result << std::endl;
        #endif

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

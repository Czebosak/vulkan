#include "vulkan_engine.hpp"

#if defined(_WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
    #define VK_USE_PLATFORM_X11_KHR   // or VK_USE_PLATFORM_WAYLAND_KHR
    #define GLFW_EXPOSE_NATIVE_X11    // or GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#include <GLFW/glfw3native.h>

#include <vk_mem_alloc.h>

#include <fmt/core.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <chrono>
#include <thread>
#include <ranges>

#include <backends/vulkan/initializers.hpp>
#include <backends/vulkan/images.hpp>
#include <backends/vulkan/pipeline_builder.hpp>

#include <voxel/render_types.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

input::Input* input_ptr;

void mouse_move_callback(GLFWwindow* window, double x_pos, double y_pos) {
    input_ptr->register_move_event(glm::vec2(x_pos, y_pos));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    input::KeyState state;
    switch (action) {
        case GLFW_RELEASE:
            state = input::KeyState::RELEASED;
            break;
        case GLFW_PRESS:
            state = input::KeyState::PRESSED;
            break;
        default: return;
    }
    
    input_ptr->register_event(static_cast<input::Key>(key), state);
}

/* AllocatedBuffer Engine::create_buffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage) {
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
    VK_CHECK(vmaCreateBuffer(allocator, &buffer_info, &vma_alloc_info, &new_buffer.buffer, &new_buffer.allocation, &new_buffer.info));

    return new_buffer;
}

void Engine::destroy_buffer(const AllocatedBuffer& buffer) {
    vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
} */

/* GPUMeshBuffers Engine::upload_mesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
    const size_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
    const size_t index_buffer_size = indices.size() * sizeof(uint32_t);

    GPUMeshBuffers new_surface;

    new_surface.vertex_buffer = AllocatedBuffer::create(vertex_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo device_adress_info{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = new_surface.vertex_buffer.buffer };
    new_surface.vertex_buffer_address = vkGetBufferDeviceAddress(device, &device_adress_info);

    new_surface.index_buffer = AllocatedBuffer::create(allocator, index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = AllocatedBuffer::create(allocator, vertex_buffer_size + index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

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

    staging.destroy(allocator);

    return new_surface;
} */

uint32_t Engine::init_vulkan(GLFWwindow *window) {
    vkb::InstanceBuilder builder;

    //make the vulkan instance, with basic debug features
    auto inst_ret = builder.set_app_name("Epic Voxel Engine")
        .request_validation_layers(ENABLE_VALIDATION_LAYERS)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    vkb::Instance vkb_inst = inst_ret.value();

    //grab the instance 
    instance = vkb_inst.instance;
    debug_messenger = vkb_inst.debug_messenger;
    
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) return -6;

    //vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features.dynamicRendering = true;
    features.synchronization2 = true;

    //vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    //use vkbootstrap to select a gpu. 
    //We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
    vkb::PhysicalDeviceSelector selector { vkb_inst };
    vkb::PhysicalDevice vkb_physical_device = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features)
        .set_required_features_12(features12)
        .set_surface(surface)
        .select()
        .value();
    
    //create the final vulkan device
    vkb::DeviceBuilder device_builder{ vkb_physical_device };

    vkb::Device vkb_device = device_builder.build().value();

    // Get the VkDevice handle used in the rest of a vulkan application
    device = vkb_device.device;
    physical_device = vkb_physical_device.physical_device;

    graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    VmaAllocatorCreateInfo allocator_info = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = physical_device,
        .device = device,
        .instance = instance,
    };

    vmaCreateAllocator(&allocator_info, &allocator);

    deletion_queue.push_function([&]() {
        vmaDestroyAllocator(allocator);
    });

    return 0;

    /* if (enable_validation_layers && !check_validation_layer_support()) {
        throw std::runtime_error("validation layers requested, but not available!");
    } */

    /* VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Epic Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    std::vector<const char*> extensions = get_required_extensions();
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    if (ENABLE_VALIDATION_LAYERS) {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();

        populate_debug_messenger_create_info(debug_create_info);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;

        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    } */
}

void Engine::init_swapchain() {
    create_swapchain(WIDTH, HEIGHT);

    VkExtent3D draw_image_extent = {
        swapchain_extent.width,
        swapchain_extent.height,
        1
    };

    //hardcoding the draw format to 32 bit float
    draw_image.image_format = VK_FORMAT_R16G16B16A16_SFLOAT;
    draw_image.image_extent = draw_image_extent;

    VkImageUsageFlags draw_image_usages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo rimg_info = vkinit::image_create_info(draw_image.image_format, draw_image_usages, draw_image_extent);

    //for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    vmaCreateImage(allocator, &rimg_info, &rimg_allocinfo, &draw_image.image, &draw_image.allocation, nullptr);

    //build a image-view for the draw image to use for rendering
    VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(draw_image.image_format, draw_image.image, VK_IMAGE_ASPECT_COLOR_BIT);

    VK_CHECK(vkCreateImageView(device, &rview_info, nullptr, &draw_image.image_view));

    //add to deletion queues
    deletion_queue.push_function([&]() {
        vkDestroyImageView(device, draw_image.image_view, nullptr);
        vmaDestroyImage(allocator, draw_image.image, draw_image.allocation);
    });
}

void Engine::create_swapchain(uint32_t width, uint32_t height) {
    vkb::SwapchainBuilder swapchain_builder{ physical_device, device, surface };

    swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkb_swapchain = swapchain_builder
        //.use_default_format_selection()
        .set_desired_format(VkSurfaceFormatKHR{ .format = swapchain_image_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        //use vsync present mode
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    swapchain_extent = vkb_swapchain.extent;
    //store swapchain and its related images
    swapchain = vkb_swapchain.swapchain;

    auto image_handles = vkb_swapchain.get_images().value();
    auto image_views = vkb_swapchain.get_image_views().value();

    swapchain_images.reserve(image_handles.size());
    
    for (const auto& [handle, image_view] : std::views::zip(image_handles, image_views)) {
        swapchain_images.emplace_back(handle, image_view);
    }
}

void Engine::init_commands() {
    //create a command pool for commands submitted to the graphics queue.
    //we also want the pool to allow for resetting of individual command buffers
    VkCommandPoolCreateInfo command_pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphics_queue_family,
    };
    
    for (int i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateCommandPool(device, &command_pool_info, nullptr, &frames[i].command_pool));

        // allocate the default command buffer that we will use for rendering
        VkCommandBufferAllocateInfo cmd_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = frames[i].command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VK_CHECK(vkAllocateCommandBuffers(device, &cmd_alloc_info, &frames[i].main_command_buffer));
    }

    VK_CHECK(vkCreateCommandPool(device, &command_pool_info, nullptr, &imm_command_pool));

    // allocate the command buffer for immediate submits
    VkCommandBufferAllocateInfo cmd_alloc_info = vkinit::command_buffer_allocate_info(imm_command_pool, 1);

    VK_CHECK(vkAllocateCommandBuffers(device, &cmd_alloc_info, &imm_command_buffer));

    deletion_queue.push_function([&]() { 
        vkDestroyCommandPool(device, imm_command_pool, nullptr);
    });
}

void Engine::init_sync_structures() {
    //create syncronization structures
    //one fence to control when the gpu has finished rendering the frame,
    //and 2 semaphores to syncronize rendering with swapchain
    //we want the fence to start signalled so we can wait on it on the first frame
    VkFenceCreateInfo fence_create_info = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphore_create_info = vkinit::semaphore_create_info();

    for (int i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateFence(device, &fence_create_info, nullptr, &frames[i].render_fence));

        VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &frames[i].swapchain_semaphore));
        VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &frames[i].render_semaphore));
    }

    for (SwapchainImage& swapchain_image : swapchain_images) {
        VK_CHECK(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &swapchain_image.submit_semaphore));
    }

    VK_CHECK(vkCreateFence(device, &fence_create_info, nullptr, &imm_fence));
    deletion_queue.push_function([&]() { vkDestroyFence(device, imm_fence, nullptr); });
}

void Engine::init_descriptors() {
    //create a descriptor pool that will hold 10 sets with 1 image each
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }};

    global_descriptor_allocator.init_pool(device, 10, sizes);

    // make the descriptor set layout for our compute draw
    {
        DescriptorLayoutBuilder builder;
        builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        draw_image_descriptor_layout = builder.build(device, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    // allocate a descriptor set for our draw image
    draw_image_descriptors = global_descriptor_allocator.allocate(device, draw_image_descriptor_layout);	

    VkDescriptorImageInfo img_info = {
        .imageView = draw_image.image_view,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };
    
    VkWriteDescriptorSet draw_image_write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = draw_image_descriptors,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &img_info,
    };

    vkUpdateDescriptorSets(device, 1, &draw_image_write, 0, nullptr);

    //make sure both the descriptor allocator and the new layout get cleaned up properly
    deletion_queue.push_function([&]() {
        global_descriptor_allocator.destroy_pool(device);

        vkDestroyDescriptorSetLayout(device, draw_image_descriptor_layout, nullptr);
    });
}

void Engine::init_background_pipelines() {
    VkPushConstantRange push_constant = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(ComputePushConstants),
    };

    VkPipelineLayoutCreateInfo compute_layout = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &draw_image_descriptor_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant,
    };

    VK_CHECK(vkCreatePipelineLayout(device, &compute_layout, nullptr, &gradient_pipeline_layout));

    VkShaderModule compute_draw_shader;
    auto compute_draw_shader_opt = vkutil::load_shader_module(device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/gradient_color.spv");
    if (compute_draw_shader_opt) {
        compute_draw_shader = *compute_draw_shader_opt;
    } else {
        fmt::println("Error when building the compute shader");
    }

    VkPipelineShaderStageCreateInfo stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = compute_draw_shader,
        .pName = "main",
    };

    VkComputePipelineCreateInfo compute_pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = stage_info,
        .layout = gradient_pipeline_layout,
    };
    
    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr, &gradient_pipeline));

    vkDestroyShaderModule(device, compute_draw_shader, nullptr);

    deletion_queue.push_function([&]() {
        vkDestroyPipelineLayout(device, gradient_pipeline_layout, nullptr);
        vkDestroyPipeline(device, gradient_pipeline, nullptr);
    });
}

void Engine::init_pipelines() {
    init_background_pipelines();

    init_triangle_pipeline();
    init_mesh_pipeline();
    init_voxel_pipeline();
}

void Engine::init_imgui() {
    // 1: create descriptor pool for IMGUI
    //  the size of the pool is very oversize, but it's copied from imgui demo
    //  itself.
    VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 },
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imgui_pool;
    VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &imgui_pool));

    // 2: initialize imgui library

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for SDL
    //ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {
        .Instance = instance,
        .PhysicalDevice = physical_device,
        .Device = device,
        .Queue = graphics_queue,
        .DescriptorPool = imgui_pool,
        .MinImageCount = 3,
        .ImageCount = 3,
        //dynamic rendering parameters for imgui to use
        .PipelineInfoMain {
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .PipelineRenderingCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &swapchain_image_format,
            },
        },
        .UseDynamicRendering = true,
    };

    ImGui_ImplVulkan_Init(&init_info);

    /* ImGui_ImplVulkan_CreateFontsTexture(); */

    // add the destroy the imgui created structures
    deletion_queue.push_function([&]() {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(device, imgui_pool, nullptr);
    });
}

void Engine::init_triangle_pipeline() {
    VkShaderModule triangle_frag_shader;
    auto triangle_frag_shader_opt = vkutil::load_shader_module(device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/colored_triangle.frag.spv");
    if (triangle_frag_shader_opt) {
        triangle_frag_shader = *triangle_frag_shader_opt;
        fmt::print("Triangle fragment shader succesfully loaded");
    } else {
        fmt::print("Error when building the triangle fragment shader module");
    }

    VkShaderModule triangle_vertex_shader;
    auto triangle_vertex_shader_opt = vkutil::load_shader_module(device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/colored_triangle.vert.spv");
    if (triangle_vertex_shader_opt) {
        triangle_vertex_shader = *triangle_vertex_shader_opt;
        fmt::print("Triangle vertex shader succesfully loaded");
    }
    else {
        fmt::print("Error when building the triangle vertex shader module");
    }
    
    //build the pipeline layout that controls the inputs/outputs of the shader
    //we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
    VkPipelineLayoutCreateInfo pipeline_layout_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

    VK_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &triangle_pipeline_layout));
    
    triangle_pipeline = hayvk::builders::PipelineBuilder { .pipeline_layout = triangle_pipeline_layout }
        .set_shaders(triangle_vertex_shader, triangle_frag_shader)
        .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .set_polygon_mode(VK_POLYGON_MODE_FILL)
        .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
        .set_multisampling_none()
        .disable_blending()
        .disable_depthtest()
        .set_color_attachment_format(draw_image.image_format)
        .set_depth_format(VK_FORMAT_UNDEFINED)
        .build(device);

    vkDestroyShaderModule(device, triangle_frag_shader, nullptr);
    vkDestroyShaderModule(device, triangle_vertex_shader, nullptr);

    deletion_queue.push_function([&]() {
        vkDestroyPipelineLayout(device, triangle_pipeline_layout, nullptr);
        vkDestroyPipeline(device, triangle_pipeline, nullptr);
    });
}

void Engine::init_mesh_pipeline() {
    VkShaderModule triangle_frag_shader;
    auto triangle_frag_shader_opt = vkutil::load_shader_module(device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/colored_triangle.frag.spv");
    if (triangle_frag_shader_opt) {
        triangle_frag_shader = *triangle_frag_shader_opt;
        fmt::print("Triangle fragment shader succesfully loaded");
    } else {
        fmt::print("Error when building the triangle fragment shader module");
    }

    VkShaderModule triangle_vertex_shader;
    auto triangle_vertex_shader_opt = vkutil::load_shader_module(device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/colored_triangle_mesh.vert.spv");
    if (triangle_vertex_shader_opt) {
        triangle_vertex_shader = *triangle_vertex_shader_opt;
        fmt::print("Triangle vertex shader succesfully loaded");
    }
    else {
        fmt::print("Error when building the triangle vertex shader module");
    }
    
    VkPushConstantRange bufferRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(GPUDrawPushConstants),
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &bufferRange,
    };

    VK_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &mesh_pipeline_layout));
    
    mesh_pipeline = hayvk::builders::PipelineBuilder { .pipeline_layout = mesh_pipeline_layout }
        .set_shaders(triangle_vertex_shader, triangle_frag_shader)
        .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .set_polygon_mode(VK_POLYGON_MODE_FILL)
        .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
        .set_multisampling_none()
        .disable_blending()
        .disable_depthtest()
        .set_color_attachment_format(draw_image.image_format)
        .set_depth_format(VK_FORMAT_UNDEFINED)
        .build(device);

    vkDestroyShaderModule(device, triangle_frag_shader, nullptr);
    vkDestroyShaderModule(device, triangle_vertex_shader, nullptr);

    deletion_queue.push_function([&]() {
        vkDestroyPipelineLayout(device, mesh_pipeline_layout, nullptr);
        vkDestroyPipeline(device, mesh_pipeline, nullptr);
    });
}

void Engine::init_voxel_pipeline() {
    VkShaderModule voxel_frag_shader;
    auto voxel_frag_shader_opt = vkutil::load_shader_module(device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/voxel.frag.spv");
    if (voxel_frag_shader_opt) {
        voxel_frag_shader = *voxel_frag_shader_opt;
        fmt::print("Voxel fragment shader succesfully loaded");
    } else {
        fmt::print("Error when building the voxel fragment shader module");
    }

    VkShaderModule voxel_vertex_shader;
    auto voxel_vertex_shader_opt = vkutil::load_shader_module(device, "/home/czebosak/Development/cpp/graphics/vulkan/assets/voxel.vert.spv");
    if (voxel_vertex_shader_opt) {
        voxel_vertex_shader = *voxel_vertex_shader_opt;
        fmt::print("Voxel vertex shader succesfully loaded");
    }
    else {
        fmt::print("Error when building the voxel vertex shader module");
    }
    
    VkPushConstantRange bufferRange = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(voxel::VoxelPushConstants),
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &bufferRange,
    };

    VK_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &voxel_pipeline_layout));
    
    voxel_pipeline = hayvk::builders::PipelineBuilder { .pipeline_layout = voxel_pipeline_layout }
        .set_shaders(voxel_vertex_shader, voxel_frag_shader)
        .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
        .set_polygon_mode(VK_POLYGON_MODE_FILL)
        .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
        .set_multisampling_none()
        .disable_blending()
        .disable_depthtest()
        .set_color_attachment_format(draw_image.image_format)
        .set_depth_format(VK_FORMAT_UNDEFINED)
        .build(device);

    vkDestroyShaderModule(device, voxel_frag_shader, nullptr);
    vkDestroyShaderModule(device, voxel_vertex_shader, nullptr);

    deletion_queue.push_function([&]() {
        vkDestroyPipelineLayout(device, voxel_pipeline_layout, nullptr);
        vkDestroyPipeline(device, voxel_pipeline, nullptr);
    });
}

void Engine::init_default_data() {
    std::array<Vertex, 8> rect_vertices = {
        Vertex { .position = glm::vec3(-0.5f, -0.5f, -0.5f) },
        Vertex { .position = glm::vec3( 0.5f, -0.5f, -0.5f) },
        Vertex { .position = glm::vec3( 0.5f,  0.5f, -0.5f) },
        Vertex { .position = glm::vec3(-0.5f,  0.5f, -0.5f) },
        Vertex { .position = glm::vec3(-0.5f, -0.5f,  0.5f) },
        Vertex { .position = glm::vec3( 0.5f, -0.5f,  0.5f) },
        Vertex { .position = glm::vec3( 0.5f,  0.5f,  0.5f) },
        Vertex { .position = glm::vec3(-0.5f,  0.5f,  0.5f) },
    };

    std::array<uint32_t, 36> rect_indices = {
        // Front face (z = +0.5)
        4, 5, 6,
        4, 6, 7,

        // Back face (z = -0.5)
        0, 2, 1,
        0, 3, 2,

        // Left face (x = -0.5)
        0, 7, 3,
        0, 4, 7,

        // Right face (x = +0.5)
        1, 2, 6,
        1, 6, 5,

        // Top face (y = +0.5)
        3, 7, 6,
        3, 6, 2,

        // Bottom face (y = -0.5)
        0, 1, 5,
        0, 5, 4,
    };

    std::span<Vertex> vspan(rect_vertices);
    std::span<uint32_t> ispan(rect_indices);

    rectangle = upload_mesh(render_state, vspan, ispan);

    deletion_queue.push_function([&](){
        rectangle.index_buffer.destroy(allocator);
        rectangle.vertex_buffer.destroy(allocator);
    });

    game_state.camera = Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        //glm::perspective(glm::radians(70.0f), 640.0f / 480.0f, 0.1f, 10000.0f)
        glm::perspective(glm::radians(70.0f), (float)swapchain_extent.width / (float)swapchain_extent.height, 0.1f, 10000.0f)
    );

    if (game_state.chunk.is_dirty()) {
        game_state.chunk.mesh_state = generate_mesh(render_state, game_state.chunk, registry::Registry::get());
    }
}

//

uint32_t Engine::init() {
    input_ptr = &input;

    static constexpr uint32_t INIT_ERR_GLFW_INIT = -1;
    static constexpr uint32_t INIT_ERR_WINDOW = -2;
    static constexpr uint32_t INIT_ERR_SURFACE = -3; 
    static constexpr uint32_t INIT_ERR_VK_INSTANCE = -4;
    static constexpr uint32_t INIT_ERR_VK_STRUCT = -5;

    int result = glfwInit();
    if (result != GLFW_TRUE) return INIT_ERR_GLFW_INIT;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan x3", nullptr, nullptr);

    if (!window) return INIT_ERR_WINDOW;

    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetKeyCallback(window, key_callback);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    init_vulkan(window);
    init_swapchain();
    init_commands();
    init_sync_structures();
    init_descriptors();

    render_state = RenderState(this, device, allocator);

    init_pipelines();
    init_imgui();
    init_default_data();

    push_constants = {
        .data1 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        .data2 = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
    };

    return 0;
}

void Engine::run() {
    bool should_close = false;
    bool minimized = false;

    while (!should_close) {
        should_close = glfwWindowShouldClose(window);
        if (should_close) break;

        input.update();

        game_state.main_loop(input);
        
        // do not draw if we are minimized
        /* if (minimized) {
            // throttle the speed
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        } */

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Epic");
            ImGui::InputFloat4("data1", (float*)&push_constants.data1);
            ImGui::InputFloat4("data2", (float*)&push_constants.data2);
            ImGui::InputFloat4("data3", (float*)&push_constants.data3);
            ImGui::InputFloat4("data4", (float*)&push_constants.data4);

            ImGui::Text("%f, %f", glm::degrees(game_state.yaw), glm::degrees(game_state.pitch));
            ImGui::Text("%f, %f, %f", game_state.camera.position.x, game_state.camera.position.y, game_state.camera.position.z);
        ImGui::End();

        ImGui::Render();

        draw();

        glfwPollEvents();
    }
}

void Engine::draw_imgui(VkCommandBuffer cmd, VkImageView target_image_view) {
    VkRenderingAttachmentInfo color_attachment = vkinit::attachment_info(target_image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo render_info = vkinit::rendering_info(swapchain_extent, &color_attachment, nullptr);

    vkCmdBeginRendering(cmd, &render_info);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}

void Engine::draw_background(VkCommandBuffer cmd) {
    //make a clear-color from frame number. This will flash with a 120 frame period.
    VkClearColorValue clear_value;
    float flash = std::abs(std::sin(frame_number / 120.f));
    clear_value = { { 0.0f, 0.0f, flash, 1.0f } };

    VkImageSubresourceRange clear_range = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    //clear image
    //vkCmdClearColorImage(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_range);

    // bind the gradient drawing compute pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline);

    // bind the descriptor set containing the draw image for the compute pipeline
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_layout, 0, 1, &draw_image_descriptors, 0, nullptr);

    /* ComputePushConstants pc;
    pc.data1 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    pc.data2 = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); */

    vkCmdPushConstants(cmd, gradient_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &push_constants);

    // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
    vkCmdDispatch(cmd, std::ceil(draw_extent.width / 16.0), std::ceil(draw_extent.height / 16.0), 1);
}

void Engine::draw_geometry(VkCommandBuffer cmd) {
    //begin a render pass  connected to our draw image
    VkRenderingAttachmentInfo color_attachment = vkinit::attachment_info(draw_image.image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkRenderingInfo render_info = vkinit::rendering_info(draw_extent, &color_attachment, nullptr);
    vkCmdBeginRendering(cmd, &render_info);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, triangle_pipeline);

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = (float)draw_extent.width,
        .height = (float)draw_extent.height,
        .minDepth = 0.f,
        .maxDepth = 1.f,
    };

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = draw_extent,
    };

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    //vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipeline);

    glm::mat4 vp = game_state.camera.get_matrix();
    glm::mat4 model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)), glm::vec3(0.0f, 0.0f, -1.0f));

    GPUDrawPushConstants push_constants;

    glm::mat4 mat = vp * model;
    push_constants.world_matrix = mat;
    push_constants.vertex_buffer = rectangle.vertex_buffer_address;

    vkCmdPushConstants(cmd, mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
    vkCmdBindIndexBuffer(cmd, rectangle.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, 36, 1, 0, 0, 0);

    vkCmdEndRendering(cmd);
}

void Engine::draw_chunk(VkCommandBuffer cmd, const voxel::Chunk& chunk) {
    //begin a render pass  connected to our draw image
    VkRenderingAttachmentInfo color_attachment = vkinit::attachment_info(draw_image.image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkRenderingInfo render_info = vkinit::rendering_info(draw_extent, &color_attachment, nullptr);
    vkCmdBeginRendering(cmd, &render_info);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, voxel_pipeline);

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = (float)draw_extent.width,
        .height = (float)draw_extent.height,
        .minDepth = 0.f,
        .maxDepth = 1.f,
    };

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = draw_extent,
    };

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, voxel_pipeline);

    glm::mat4 vp = game_state.camera.get_matrix();
    //glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    voxel::VoxelPushConstants push_constants;

    const voxel::Mesh& mesh = std::get<voxel::Mesh>(chunk.mesh_state);

    glm::mat4 mat = vp;
    push_constants.mvp = mat;
    push_constants.face_buffer = mesh.buffer_addr;
    vkCmdPushConstants(cmd, voxel_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(voxel::VoxelPushConstants), &push_constants);

    vkCmdDraw(cmd, 4, mesh.instance_count, 0, 0);

    vkCmdEndRendering(cmd);
}

void Engine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function) {
    VK_CHECK(vkResetFences(device, 1, &imm_fence));
    VK_CHECK(vkResetCommandBuffer(imm_command_buffer, 0));

    VkCommandBuffer cmd = imm_command_buffer;

    VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);
    VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, nullptr, nullptr);

    // submit command buffer to the queue and execute it.
    //  _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(graphics_queue, 1, &submit, imm_fence));

    VK_CHECK(vkWaitForFences(device, 1, &imm_fence, true, 9999999999));
}

void Engine::draw() {
    FrameData& current_frame = get_current_frame();

    // wait until the gpu has finished rendering the last frame. Timeout of 1 second
    VK_CHECK(vkWaitForFences(device, 1, &current_frame.render_fence, true, 1000000000));

    current_frame.deletion_queue.flush();

    // request image from the swapchain
    uint32_t image_index;
    VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, current_frame.swapchain_semaphore, nullptr, &image_index));

    VK_CHECK(vkResetFences(device, 1, &current_frame.render_fence));

    VK_CHECK(vkResetCommandBuffer(current_frame.main_command_buffer, 0));

    VkCommandBuffer cmd = current_frame.main_command_buffer;

    VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    draw_extent.width = draw_image.image_extent.width;
    draw_extent.height = draw_image.image_extent.height;

    VkSemaphore& submit_semaphore = swapchain_images[image_index].submit_semaphore;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));

    // transition our main draw image into general layout so we can write into it
    // we will overwrite it all so we dont care about what was the older layout
    vkutil::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    draw_background(cmd);

    vkutil::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    //draw_geometry(cmd);
    draw_chunk(cmd, game_state.chunk);

    //transition the draw image and the swapchain image into their correct transfer layouts
    vkutil::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkutil::transition_image(cmd, swapchain_images[image_index].handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // execute a copy from the draw image into the swapchain
    vkutil::copy_image_to_image(cmd, draw_image.image, swapchain_images[image_index].handle, draw_extent, swapchain_extent);

    vkutil::transition_image(cmd, swapchain_images[image_index].handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    //draw imgui into the swapchain image
    draw_imgui(cmd, swapchain_images[image_index].view);

    // set swapchain image layout to Present so we can show it on the screen
    vkutil::transition_image(cmd, swapchain_images[image_index].handle, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

    //prepare the submission to the queue. 
    //we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
    //we will signal the _renderSemaphore, to signal that rendering has finished
    VkCommandBufferSubmitInfo cmd_info = vkinit::command_buffer_submit_info(cmd);	
    
    VkSemaphoreSubmitInfo wait_info = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, current_frame.swapchain_semaphore);
    VkSemaphoreSubmitInfo signal_info = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, submit_semaphore);
    
    VkSubmitInfo2 submit = vkinit::submit_info(&cmd_info, &signal_info, &wait_info);

    //submit command buffer to the queue and execute it.
    // _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(graphics_queue, 1, &submit, current_frame.render_fence));

    //prepare present
    // this will put the image we just rendered to into the visible window.
    // we want to wait on the _renderSemaphore for that, 
    // as its necessary that drawing commands have finished before the image is displayed to the user
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &submit_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &image_index,
    };

    VK_CHECK(vkQueuePresentKHR(graphics_queue, &present_info));

    frame_number++;
}

void Engine::clean_up() {
    vkDeviceWaitIdle(device);

    deletion_queue.flush();

    for (int i = 0; i < FRAME_OVERLAP; i++) {
        //already written from before
        vkDestroyCommandPool(device, frames[i].command_pool, nullptr);

        //destroy sync objects
        vkDestroyFence(device, frames[i].render_fence, nullptr);
        vkDestroySemaphore(device, frames[i].render_semaphore, nullptr);
        vkDestroySemaphore(device, frames[i].swapchain_semaphore, nullptr);
        vkDestroySemaphore(device, frames[i].acquire_semaphore, nullptr);

        frames[i].deletion_queue.flush();
    }

    destroy_swapchain();

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);

    vkb::destroy_debug_utils_messenger(instance, debug_messenger);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Engine::destroy_swapchain() {
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    for (int i = 0; i < swapchain_images.size(); i++) {
        vkDestroyImageView(device, swapchain_images[i].view, nullptr);
        vkDestroySemaphore(device, swapchain_images[i].submit_semaphore, nullptr);
    }
}

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

#include <backends/vulkan/initializers.hpp>
#include <backends/vulkan/images.hpp>
#include <backends/vulkan/pipeline_builder.hpp>

uint32_t Engine::init_vulkan(GLFWwindow* window) {
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
	swapchain_images = vkb_swapchain.get_images().value();
	swapchain_image_views = vkb_swapchain.get_image_views().value();
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

	VK_CHECK(vkCreateFence(device, &fence_create_info, nullptr, &imm_fence));
	deletion_queue.push_function([&]() { vkDestroyFence(device, imm_fence, nullptr); });
}

void Engine::init_descriptors() {
	//create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }};

	global_descriptor_allocator.init_pool(device, 10, sizes);

	//make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		draw_image_descriptor_layout = builder.build(device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	//allocate a descriptor set for our draw image
	draw_image_descriptors = global_descriptor_allocator.allocate(device, draw_image_descriptor_layout);	

	VkDescriptorImageInfo imgInfo = {
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
		.pImageInfo = &imgInfo,
	};

	vkUpdateDescriptorSets(device, 1, &draw_image_write, 0, nullptr);

	//make sure both the descriptor allocator and the new layout get cleaned up properly
	deletion_queue.push_function([&]() {
		global_descriptor_allocator.destroy_pool(device);

		vkDestroyDescriptorSetLayout(device, draw_image_descriptor_layout, nullptr);
	});
}

void Engine::init_background_pipelines() {
	VkPipelineLayoutCreateInfo compute_layout = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &draw_image_descriptor_layout,
	};

	VK_CHECK(vkCreatePipelineLayout(device, &compute_layout, nullptr, &gradient_pipeline_layout));

	VkShaderModule compute_draw_shader;
	auto compute_draw_shader_opt = vkutil::load_shader_module(device, "assets/gradient.spv");
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

uint32_t Engine::init() {
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

    init_vulkan(window);
	init_swapchain();
	init_commands();
	init_sync_structures();
	init_descriptors();
	init_pipelines();
	init_imgui();

    return 0;
}

void Engine::run() {
    bool should_close = false;
    bool minimized = false;

    while (!should_close) {
        should_close = glfwWindowShouldClose(window);
        if (should_close) break;
        
        // do not draw if we are minimized
        if (minimized) {
            // throttle the speed
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();

        draw();
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

	// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
	vkCmdDispatch(cmd, std::ceil(draw_extent.width / 16.0), std::ceil(draw_extent.height / 16.0), 1);
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
	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(device, 1, &get_current_frame().render_fence, true, 1000000000));

	get_current_frame().deletion_queue.flush();
	//VK_CHECK(vkResetFences(device, 1, &get_current_frame().render_fence));

    // request image from the swapchain
	uint32_t swapchain_image_index;
	VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, get_current_frame().swapchain_semaphore, nullptr, &swapchain_image_index));

	VK_CHECK(vkResetFences(device, 1, &get_current_frame().render_fence));

	VK_CHECK(vkResetCommandBuffer(get_current_frame().main_command_buffer, 0));

	VkCommandBuffer cmd = get_current_frame().main_command_buffer;

	//VK_CHECK(vkResetCommandBuffer(cmd, 0));

	VkCommandBufferBeginInfo cmd_begin_info = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	draw_extent.width = draw_image.image_extent.width;
	draw_extent.height = draw_image.image_extent.height;

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmd_begin_info));	

	// transition our main draw image into general layout so we can write into it
	// we will overwrite it all so we dont care about what was the older layout
	vkutil::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	draw_background(cmd);

	//transition the draw image and the swapchain image into their correct transfer layouts
	vkutil::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	vkutil::transition_image(cmd, swapchain_images[swapchain_image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// execute a copy from the draw image into the swapchain
	vkutil::copy_image_to_image(cmd, draw_image.image, swapchain_images[swapchain_image_index], draw_extent, swapchain_extent);

	vkutil::transition_image(cmd, swapchain_images[swapchain_image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	//draw imgui into the swapchain image
	draw_imgui(cmd, swapchain_image_views[swapchain_image_index]);

	// set swapchain image layout to Present so we can show it on the screen
	vkutil::transition_image(cmd, swapchain_images[swapchain_image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished
	VkCommandBufferSubmitInfo cmd_info = vkinit::command_buffer_submit_info(cmd);	
	
	VkSemaphoreSubmitInfo wait_info = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame().swapchain_semaphore);
	VkSemaphoreSubmitInfo signal_info = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, get_current_frame().render_semaphore);	
	
	VkSubmitInfo2 submit = vkinit::submit_info(&cmd_info, &signal_info, &wait_info);	

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(graphics_queue, 1, &submit, get_current_frame().render_fence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &get_current_frame().render_semaphore,
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &swapchain_image_index,
	};

	VK_CHECK(vkQueuePresentKHR(graphics_queue, &present_info));

	frame_number++;
}

void Engine::clean_up() {
	vkDeviceWaitIdle(device);

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		//already written from before
		vkDestroyCommandPool(device, frames[i].command_pool, nullptr);

		//destroy sync objects
		vkDestroyFence(device, frames[i].render_fence, nullptr);
		vkDestroySemaphore(device, frames[i].render_semaphore, nullptr);
		vkDestroySemaphore(device ,frames[i].swapchain_semaphore, nullptr);
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

	for (int i = 0; i < swapchain_image_views.size(); i++) {
		vkDestroyImageView(device, swapchain_image_views[i], nullptr);
	}
}

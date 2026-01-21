#include "descriptors.hpp"

DescriptorLayoutBuilder& DescriptorLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType type) {
    VkDescriptorSetLayoutBinding newbind {};
    newbind.binding = binding;
    newbind.descriptorCount = 1;
    newbind.descriptorType = type;

    bindings.push_back(newbind);

    return *this;
}

void DescriptorLayoutBuilder::clear() {
    bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shader_stages, void* p_next, VkDescriptorSetLayoutCreateFlags flags) {
    for (auto& b : bindings) {
        b.stageFlags |= shader_stages;
    }

    VkDescriptorSetLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = p_next,
        .flags = flags,
        .bindingCount = (uint32_t)bindings.size(),
        .pBindings = bindings.data(),
    };

    VkDescriptorSetLayout set;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

    return set;
}

void DescriptorAllocator::init_pool(VkDevice device, uint32_t max_sets, std::span<PoolSizeRatio> pool_ratios) {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (PoolSizeRatio ratio : pool_ratios) {
        pool_sizes.push_back(VkDescriptorPoolSize{
            .type = ratio.type,
            .descriptorCount = uint32_t(ratio.ratio * max_sets)
        });
    }

	VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = max_sets,
        .poolSizeCount = (uint32_t)pool_sizes.size(),
        .pPoolSizes = pool_sizes.data(),
    };

	VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &pool));
}

void DescriptorAllocator::clear_descriptors(VkDevice device) {
    vkResetDescriptorPool(device, pool, 0);
}

void DescriptorAllocator::destroy_pool(VkDevice device) {
    vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };

    VkDescriptorSet ds;
    VK_CHECK(vkAllocateDescriptorSets(device, &alloc_info, &ds));

    return ds;
}

void DescriptorAllocatorGrowable::init(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
{
    ratios.clear();
    
    for (auto r : poolRatios) {
        ratios.push_back(r);
    }
	
    VkDescriptorPool newPool = create_pool(device, maxSets, poolRatios);

    setsPerPool = maxSets * 1.5; //grow it next allocation

    readyPools.push_back(newPool);
}

void DescriptorAllocatorGrowable::clear_pools(VkDevice device)
{ 
    for (auto p : readyPools) {
        vkResetDescriptorPool(device, p, 0);
    }
    for (auto p : fullPools) {
        vkResetDescriptorPool(device, p, 0);
        readyPools.push_back(p);
    }
    fullPools.clear();
}

void DescriptorAllocatorGrowable::destroy_pools(VkDevice device)
{
	for (auto p : readyPools) {
		vkDestroyDescriptorPool(device, p, nullptr);
	}
    readyPools.clear();
	for (auto p : fullPools) {
		vkDestroyDescriptorPool(device,p,nullptr);
    }
    fullPools.clear();
}

VkDescriptorPool DescriptorAllocatorGrowable::get_pool(VkDevice device) {
    VkDescriptorPool new_pool;
    if (ready_pools.size() != 0) {
        new_pool = ready_pools.back();
        ready_pools.pop_back();
    }
    else {
	    //need to create a new pool
	    new_pool = create_pool(device, sets_per_pool, ratios);

	    sets_per_pool = sets_per_pool * 1.5;
	    if (sets_per_pool > 4092) {
		    sets_per_pool = 4092;
	    }
    }   

    return new_pool;
}

VkDescriptorPool DescriptorAllocatorGrowable::create_pool(VkDevice device, uint32_t set_count, std::span<PoolSizeRatio> pool_ratios) {
	std::vector<VkDescriptorPoolSize> pool_sizes;
	for (PoolSizeRatio ratio : pool_ratios) {
		pool_sizes.push_back(VkDescriptorPoolSize{
			.type = ratio.type,
			.descriptorCount = uint32_t(ratio.ratio * set_count)
		});
	}

	VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = set_count,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

	VkDescriptorPool new_pool;
	vkCreateDescriptorPool(device, &pool_info, nullptr, &new_pool);
    return new_pool;
}

void DescriptorWriter::write_buffer(uint32_t binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) {
	VkDescriptorBufferInfo& info = buffer_infos.emplace_back(VkDescriptorBufferInfo {
		.buffer = buffer,
		.offset = offset,
		.range = size
    });

	VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstBinding = binding,
	    .dstSet = VK_NULL_HANDLE,
	    .descriptorCount = 1,
	    .descriptorType = type,
	    .pBufferInfo = &info,
    };

	writes.push_back(write);
}

void DescriptorWriter::write_image(uint32_t binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type) {
    VkDescriptorImageInfo& info = image_infos.emplace_back(VkDescriptorImageInfo {
		.sampler = sampler,
		.imageView = image,
		.imageLayout = layout
	});

	VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
	    .dstBinding = binding,
	    .dstSet = VK_NULL_HANDLE,
	    .descriptorCount = 1,
	    .descriptorType = type,
	    .pImageInfo = &info,
    };

	writes.push_back(write);
}

void DescriptorWriter::clear()
{
    image_infos.clear();
    writes.clear();
    buffer_infos.clear();
}

void DescriptorWriter::update_set(VkDevice device, VkDescriptorSet set) {
    for (VkWriteDescriptorSet& write : writes) {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

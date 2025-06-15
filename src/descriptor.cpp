#include <wil/descriptor.hpp>
#include <wil/log.hpp>
#include <wil/pipeline.hpp>

#include <numeric>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

DescriptorPool::DescriptorPool(Pipeline &pipeline, const std::vector<uint32_t> &max_sets)
	: device_(pipeline.GetDevice()), layouts_(pipeline.GetDescriptorSetLayouts())
{
	std::array<uint32_t, WIL_DESCRIPTOR_TYPE_ENUM_MAX> descriptor_count = {0, 0, 0, 0};

	for (int i = 0; i < layouts_.size(); ++i)
	{
		auto &c = layouts_[i].descriptor_count_;
		for (int j = 0; j < WIL_DESCRIPTOR_TYPE_ENUM_MAX; ++j) {
			descriptor_count[j] += c[j] * max_sets[i];
		}
	}

	constexpr VkDescriptorType types[] = {
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
	};

	std::vector<VkDescriptorPoolSize> pool_sizes;

	for (int i = 0; i < WIL_DESCRIPTOR_TYPE_ENUM_MAX; ++i)
	{
		if (!descriptor_count[i]) continue;
		pool_sizes.push_back({types[i], descriptor_count[i]});
	}

	VkDescriptorPoolCreateInfo pool_i{};
	pool_i.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_i.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_i.pPoolSizes = pool_sizes.data();
	pool_i.maxSets = std::accumulate(max_sets.begin(), max_sets.end(), 0);

	auto device = static_cast<VkDevice>(device_.GetVkDevicePtr_());

	VkDescriptorPool pool;
	if (vkCreateDescriptorPool(device, &pool_i, nullptr, &pool) != VK_SUCCESS)
		WIL_LOGERROR("Unable to create descriptor pool");
	pool_ptr_ = pool;
}

DescriptorPool::~DescriptorPool()
{
	auto device = static_cast<VkDevice>(device_.GetVkDevicePtr_());
	vkDestroyDescriptorPool(device, static_cast<VkDescriptorPool>(pool_ptr_), nullptr);
}

void DescriptorPool::AllocateSets(uint32_t set, DescriptorSet *outptr, uint32_t count)
{
	auto device = static_cast<VkDevice>(device_.GetVkDevicePtr_());

	std::vector<VkDescriptorSetLayout> layouts(count,
			static_cast<VkDescriptorSetLayout>(layouts_[set].descriptor_set_layout_ptr_));

	VkDescriptorSetAllocateInfo desc_set_ai{};
	desc_set_ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	desc_set_ai.descriptorPool = static_cast<VkDescriptorPool>(pool_ptr_);
	desc_set_ai.descriptorSetCount = count;
	desc_set_ai.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> sets;
	sets.resize(count);
	if (vkAllocateDescriptorSets(device, &desc_set_ai, sets.data()) != VK_SUCCESS)
		WIL_LOGERROR("Unable to allocate descriptor sets");

	for (int i = 0; i < sets.size(); ++i)
		outptr[i] = DescriptorSet(device_, sets[i]);
}

void DescriptorPool::Reset()
{
	auto device = static_cast<VkDevice>(device_.GetVkDevicePtr_());
	vkResetDescriptorPool(device, static_cast<VkDescriptorPool>(pool_ptr_), 0);
}

DescriptorSet::DescriptorSet(Device &device, VendorPtr vkdescriptorset)
	: device_(&device), descriptor_set_ptr_(vkdescriptorset)
{
}
	
void DescriptorSet::BindUniform(uint32_t binding, UniformBuffer &buffer)
{
	VkDescriptorBufferInfo bi{};
	bi.buffer = static_cast<VkBuffer>(buffer.GetVkBufferPtr_());
	bi.offset = 0;
	bi.range = buffer.GetSize();

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = static_cast<VkDescriptorSet>(descriptor_set_ptr_);
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bi;

    vkUpdateDescriptorSets(static_cast<VkDevice>(device_->GetVkDevicePtr_()), 1, &write, 0, nullptr);
}

void DescriptorSet::BindStorage(uint32_t binding, StorageBuffer &buffer)
{
	VkDescriptorBufferInfo bi{};
	bi.buffer = static_cast<VkBuffer>(buffer.GetVkBufferPtr_());
	bi.offset = 0;
	bi.range = buffer.GetSize();

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = static_cast<VkDescriptorSet>(descriptor_set_ptr_);
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bi;

    vkUpdateDescriptorSets(static_cast<VkDevice>(device_->GetVkDevicePtr_()), 1, &write, 0, nullptr);
}

void DescriptorSet::BindTexture(uint32_t binding, const Texture &texture)
{
	VkDescriptorImageInfo ii{};
	ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ii.imageView = static_cast<VkImageView>(texture.GetVkImageViewPtr_());
	ii.sampler = static_cast<VkSampler>(texture.GetVkSamplerPtr_());

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = static_cast<VkDescriptorSet>(descriptor_set_ptr_);
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &ii;

    vkUpdateDescriptorSets(static_cast<VkDevice>(device_->GetVkDevicePtr_()), 1, &write, 0, nullptr);
}

}

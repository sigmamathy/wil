#pragma once

#include "pipeline.hpp"
#include "buffer.hpp"

namespace wil {

class DescriptorPool
{
public:

	DescriptorPool(Pipeline &pipeline, const std::vector<uint32_t> &max_sets);

	~DescriptorPool();

	std::vector<DescriptorSet> AllocateSets(uint32_t set, uint32_t count);

	void Reset();

private:

	Device &device_;
	const std::vector<DescriptorSetLayout> &layouts_;
	VendorPtr pool_ptr_;
};

class DescriptorSet
{
public:

	DescriptorSet(std::nullptr_t);

	DescriptorSet(Device &device, VendorPtr vkdescriptorset);
	
	VendorPtr GetVkDescriptorSetPtr_() { return descriptor_set_ptr_; }

	void BindUniform(uint32_t binding, UniformBuffer &buffer);

	void BindTexture(uint32_t binding, Texture &texture);

private:
	Device *device_;
	VendorPtr descriptor_set_ptr_;
};

}

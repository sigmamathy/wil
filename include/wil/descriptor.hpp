#pragma once

#include "pipeline.hpp"
#include "buffer.hpp"

namespace wil {

class DescriptorSet
{
public:

	DescriptorSet() : descriptor_set_ptr_(nullptr) {}

	DescriptorSet(Device &device, VendorPtr vkdescriptorset);
	
	VendorPtr GetVkDescriptorSetPtr_() { return descriptor_set_ptr_; }

	void BindUniform(uint32_t binding, UniformBuffer &buffer);

	void BindStorage(uint32_t binding, StorageBuffer &buffer);

	void BindTexture(uint32_t binding, const Texture &texture);

private:
	Device *device_;
	VendorPtr descriptor_set_ptr_;
};

class DescriptorPool
{
public:

	DescriptorPool(Pipeline &pipeline, const std::vector<uint32_t> &max_sets);

	~DescriptorPool();

	void AllocateSets(uint32_t set, DescriptorSet *outptr, uint32_t count);

	void Reset();

private:

	Device &device_;
	const std::vector<DescriptorSetLayout> &layouts_;
	VendorPtr pool_ptr_;
};

template<class T>
constexpr size_t std140_alignment()
{
	if (std::is_same_v<T, bool>)
		return 1;

	if (std::is_same_v<T, float>
			|| std::is_same_v<T, int>
			|| std::is_same_v<T, unsigned>)
		return 4;

	if (std::is_same_v<T, Fvec2>
			|| std::is_same_v<T, Ivec2>
			|| std::is_same_v<T, Uvec2>)
		return 8;

	if (std::is_same_v<T, Fvec3>
			|| std::is_same_v<T, Ivec3>
			|| std::is_same_v<T, Uvec3>)
		return 16;

	if (std::is_same_v<T, Fvec4>
			|| std::is_same_v<T, Ivec4>
			|| std::is_same_v<T, Uvec4>)
		return 16;

	if (std::is_same_v<T, Fmat2>)
		return 8;

	if (std::is_same_v<T, Fmat3>
			|| std::is_same_v<T, Fmat4>)
		return 16;

	WIL_UNREACHABLE;
}

#define WIL_ALIGN_STD140(t) alignas(std140_alignment<t>()) t

}

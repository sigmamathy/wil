#pragma once

#include "device.hpp"
#include "buffer.hpp"
#include <utility>
#include <cstdint>
#include <vector>
#include <memory>

namespace wil {

enum ShaderType
{
	VERTEX_SHADER,
	FRAGMENT_SHADER,
};

struct VertexAttribLayout
{
	uint32_t location;
	uint32_t binding;
	uint32_t format;
	uint32_t offset;
};

template<class T>
uint32_t getvkattribformat_();

#define wilvrta(l, cn, m) VertexAttribLayout{(l), 0, getvkattribformat_<decltype(std::declval<cn>().m)>(), offsetof(cn, m)}

enum DescriptorType {
	UNIFORM_BUFFER,
};

struct DescriptorSetLayout
{
	struct Binding {
		DescriptorType type;
		uint32_t binding;
		ShaderType stage;
		size_t size;
	};

	std::vector<Binding> bindings;
};

class DescriptorSet
{
public:
	DescriptorSet(Device &device, VendorPtr vkdescriptorset, const DescriptorSetLayout &layout);
	
	UniformBuffer &GetUniform(uint32_t binding) { return buffers_[binding]; }

	VendorPtr GetVkDescriptorSetPtr_() { return descriptor_set_ptr_; }

private:
	VendorPtr descriptor_set_ptr_;
	std::vector<UniformBuffer> buffers_;
};

struct PipelineCtor
{
	Device *device;
	char const *shaders[2];
	std::vector<VertexAttribLayout> vertex_layout;
	uint32_t vertex_stride;
	std::vector<DescriptorSetLayout> descriptor_set_layouts;
};

class Pipeline
{
public:
	Pipeline(const PipelineCtor &ctor);
	~Pipeline();
	WIL_DELETE_COPY_AND_REASSIGNMENT(Pipeline);

	std::vector<DescriptorSet> CreateDescriptorSets(const std::vector<uint32_t> &set_ids);

	size_t GetDescriptorSetsCount() const { return descriptor_set_layouts_ptr_.size(); }

	VendorPtr GetVkPipelinePtr_() { return pipeline_ptr_; }

	VendorPtr GetVkPipelineLayoutPtr_() { return layout_ptr_; }

private:

	Device &device_;
	VendorPtr pipeline_ptr_, layout_ptr_;

	std::vector<DescriptorSetLayout> set_layouts_;
	std::vector<VendorPtr> descriptor_set_layouts_ptr_;
	VendorPtr descriptor_pool_ptr_;
};

}

#pragma once

#include "device.hpp"
#include <utility>
#include <cstdint>
#include <vector>

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

enum DescriptorType
{
	UNIFORM_BUFFER,
	COMBINED_IMAGE_SAMPLER
};

struct DescriptorSetLayout
{
	struct Binding {
		uint32_t binding;
		DescriptorType type;
		ShaderType stage;
	};

	void Add(uint32_t binding, DescriptorType type, ShaderType stage);

	std::vector<Binding> bindings_;
	std::array<uint32_t, 2> descriptor_count_ = {0, 0};
	VendorPtr descriptor_set_layout_ptr_;
};

struct PipelineCtor
{
	Device *device;
	char const *shaders[2];
	std::vector<VertexAttribLayout> vertex_layout;
	uint32_t vertex_stride;
	std::vector<DescriptorSetLayout> descriptor_set_layouts;
	bool depth_test = true;
};

class Pipeline
{
public:

	Pipeline(const PipelineCtor &ctor);

	~Pipeline();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Pipeline);

	const std::vector<DescriptorSetLayout> &GetDescriptorSetLayouts() const { return descriptor_set_layouts_; }

	Device &GetDevice() { return device_; }

	VendorPtr GetVkPipelinePtr_() { return pipeline_ptr_; }

	VendorPtr GetVkPipelineLayoutPtr_() { return layout_ptr_; }

private:

	Device &device_;
	VendorPtr pipeline_ptr_, layout_ptr_;

	std::vector<DescriptorSetLayout> descriptor_set_layouts_;
};

}

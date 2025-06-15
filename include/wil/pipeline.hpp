#pragma once

#include "device.hpp"
#include <utility>
#include <cstdint>
#include <vector>

namespace wil {

enum ShaderStageBit
{
	VERTEX_SHADER		= 1,
	FRAGMENT_SHADER		= 2,
};

WIL_ENUM_DEFINE_OR_OPERATOR(ShaderStageBit);

struct VertexAttribLayout
{
	uint32_t location;
	uint32_t binding;
	uint32_t format;
	uint32_t offset;
};

template<class T>
uint32_t getvkattribformat_();

#define wilvrta(l, cn, m) ::wil::VertexAttribLayout{(l), 0, ::wil::getvkattribformat_<decltype(std::declval<cn>().m)>(), offsetof(cn, m)}

enum DescriptorType
{
	UNIFORM_BUFFER,
	UNIFORM_BUFFER_DYNAMIC,
	STORAGE_BUFFER,
	COMBINED_IMAGE_SAMPLER,
};

#define WIL_DESCRIPTOR_TYPE_ENUM_MAX 4

struct DescriptorSetLayout
{
	struct Binding {
		uint32_t binding;
		DescriptorType type;
		ShaderStageBit stage;
	};

	void Add(uint32_t binding, DescriptorType type, ShaderStageBit stage);

	std::vector<Binding> bindings_;
	std::array<uint32_t, WIL_DESCRIPTOR_TYPE_ENUM_MAX> descriptor_count_ = {0, 0, 0, 0};
	VendorPtr descriptor_set_layout_ptr_;
};

struct PipelineCtor
{
	Device *device;
	char const *vertex_shader;
	char const *fragment_shader;

	std::vector<VertexAttribLayout> vertex_layout;
	uint32_t vertex_stride;

	ShaderStageBit push_constant_stage;
	size_t push_constant_size = 0;

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

	ShaderStageBit GetPushConstantStage() const { return push_constant_stage_; }

	size_t GetPushConstantSize() const { return push_constant_size_; }

	VendorPtr GetVkPipelinePtr_() { return pipeline_ptr_; }

	VendorPtr GetVkPipelineLayoutPtr_() { return layout_ptr_; }

private:

	Device &device_;
	VendorPtr pipeline_ptr_, layout_ptr_;

	ShaderStageBit push_constant_stage_;
	size_t push_constant_size_;

	std::vector<DescriptorSetLayout> descriptor_set_layouts_;
};

}

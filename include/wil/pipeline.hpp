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

struct PipelineCtor
{
	Device *device;
	char const *shaders[2];
	std::vector<VertexAttribLayout> vertex_layout;
	uint32_t vertex_stride;
};

class Pipeline
{
public:
	Pipeline(const PipelineCtor &ctor);
	~Pipeline();
	WIL_DELETE_COPY_AND_REASSIGNMENT(Pipeline);

	VendorPtr GetVkPipelinePtr_() { return pipeline_ptr_; }

private:

	VendorPtr vkdevice_;
	VendorPtr pipeline_ptr_, layout_ptr_;
};

}

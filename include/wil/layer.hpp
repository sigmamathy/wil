#pragma once

#include "pipeline.hpp"
#include "device.hpp"
#include <string>
#include <memory>

namespace wil {

class Layer
{
public:

	Layer(Device &dev, const PipelineCtor &ctor);

	virtual ~Layer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Layer);
	
	virtual CommandBuffer &Render(uint32_t frame, uint32_t index) = 0;

	virtual std::string GetName() const = 0;

	Pipeline &GetPipeline() { return *pipeline_; }

	CommandBuffer &GetCommandBuffer(uint32_t frame) { return cmd_buffers_[frame]; }

private:
	std::unique_ptr<Pipeline> pipeline_;
	std::vector<CommandBuffer> cmd_buffers_;
};

struct Vertex3D
{
	Fvec2 pos;
};

struct MVP3D
{
	Fmat4 model;
	Fmat4 view;
	Fmat4 proj;
};

class Layer3D : public Layer
{
public:

	Layer3D(Device &device);

	~Layer3D();

	static PipelineCtor MakePipeline(Device &device);

	DescriptorSet &GetDescriptorSet(uint32_t frame, uint32_t set) { return descriptor_sets_[set + frame]; }

private:
	std::vector<DescriptorSet> descriptor_sets_;
};

}

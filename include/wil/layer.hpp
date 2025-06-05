#pragma once

#include "pipeline.hpp"
#include "device.hpp"
#include <string>
#include <memory>

namespace wil {

class Layer
{
public:

	Layer() = default;

	virtual ~Layer() = default;

	WIL_DELETE_COPY_AND_REASSIGNMENT(Layer);
	
	void Init(Device &device);

	void Free();

	virtual CommandBuffer &Render(uint32_t frame, uint32_t index) = 0;

	virtual void OnInit(Device &device) {}

	virtual void OnClose() {}

	virtual std::unique_ptr<Pipeline> MakePipeline(Device &device) = 0;

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

	void OnInit(Device &device) override;

	void OnClose() override;

	std::unique_ptr<Pipeline> MakePipeline(Device &device) override;

	DescriptorSet &GetDescriptorSet(uint32_t frame, uint32_t set) { return descriptor_sets_[set + frame]; }

private:
	std::vector<DescriptorSet> descriptor_sets_;
};

}

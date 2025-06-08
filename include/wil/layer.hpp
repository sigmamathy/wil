#pragma once

#include "pipeline.hpp"
#include "device.hpp"
#include "cmdbuf.hpp"
#include <string>
#include <memory>

namespace wil {

class Layer
{
public:

	Layer(Device &dev);

	virtual ~Layer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Layer);
	
	virtual CommandBuffer &Render(uint32_t frame, uint32_t index) = 0;

	virtual std::string GetName() const = 0;

	CommandBuffer &GetCommandBuffer(uint32_t frame) { return cmd_buffers_[frame]; }

private:
	std::vector<CommandBuffer> cmd_buffers_;
};

struct Vertex3D
{
	Fvec3 pos;
	Fvec2 texcoord;
};

struct MVP3D
{
	Fmat4 view;
	Fmat4 proj;
};

class Layer3D : public Layer
{
public:

	Layer3D(Device &device);

	~Layer3D();

	Pipeline &GetPipeline() { return *pipeline_; }

private:
	std::unique_ptr<Pipeline> pipeline_;
};

}

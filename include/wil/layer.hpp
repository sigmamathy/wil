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
	Fvec3 normal;
};

struct LightVertex3D
{
	Fvec3 pos;
};

struct LightPushConstant3D
{
	Fmat4 model;
	Fvec4 light_color; 
};

struct GlobalData3D
{
	alignas(16) Fmat4 view;
	alignas(16) Fmat4 proj;
	alignas(16) Fvec3 view_pos;
};

class Layer3D : public Layer
{
public:

	Layer3D(Device &device);

	~Layer3D();

	Pipeline &GetPipeline() { return *pipeline_; }

	Pipeline &GetLightPipeline() { return *light_pipeline_; }

private:
	std::unique_ptr<Pipeline> pipeline_, light_pipeline_;
};

}

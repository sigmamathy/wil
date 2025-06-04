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

	CommandBuffer &Render(uint32_t frame, uint32_t index);

	virtual void OnInit(Device &device) {}

	virtual void OnClose() {}

	virtual void OnRender(CommandBuffer &cmd, uint32_t index) = 0;

	virtual std::unique_ptr<Pipeline> MakePipeline(Device &device) = 0;

	virtual std::string GetName() const = 0;

	Pipeline &GetPipeline() { return *pipeline_; }

private:
	std::unique_ptr<Pipeline> pipeline_;
	std::vector<CommandBuffer> cmd_buffers_;
};

struct Vertex3D
{
	Fvec2 pos;
};

class Layer3D : public Layer
{
public:

	void OnInit(Device &device) override;

	void OnClose() override;

	void OnRender(CommandBuffer &cmd, uint32_t index) override;

	std::unique_ptr<Pipeline> MakePipeline(Device &device) override;
};

}

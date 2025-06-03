#include <wil/layer.hpp>

namespace wil {

void Layer::Init(Device &device)
{
	pipeline_ = MakePipeline(device);
	OnInit(device);
}

void Layer::Free()
{
	OnClose();
	pipeline_.reset();
}

void Layer3D::OnInit(Device &device)
{
	cmd_buffer_ = new CommandBuffer(device);
}

void Layer3D::OnClose()
{
	delete cmd_buffer_;
}

CommandBuffer &Layer3D::Render(uint32_t index)
{
	cmd_buffer_->Reset();

	cmd_buffer_->RecordDraw(0, [this](CmdDraw &cmd){
		cmd.BindPipeline(GetPipeline());
	});

	return *cmd_buffer_;
}

std::unique_ptr<Pipeline> Layer3D::MakePipeline(Device &device)
{
	PipelineCtor ctor;
	ctor.device = &device;
	ctor.shaders[VERTEX_SHADER] = "../shaders/3d.vert.spv";
	ctor.shaders[FRAGMENT_SHADER] = "../shaders/3d.frag.spv";
	ctor.vertex_layout.push_back(wilvrta(0, Vertex3D, pos));
	ctor.vertex_stride = sizeof(Vertex3D);
	
	return std::make_unique<Pipeline>(ctor);
}

}

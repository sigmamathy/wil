#include <wil/layer.hpp>
#include <wil/app.hpp>
#include <wil/buffer.hpp>

namespace wil {

void Layer::Init(Device &device)
{
	pipeline_ = MakePipeline(device);
	for (uint32_t i = 0; i < App::Instance()->GetFramesInFlight(); ++i)
		cmd_buffers_.emplace_back(device);
	OnInit(device);
}

void Layer::Free()
{
	OnClose();
	pipeline_.reset();
}

CommandBuffer &Layer::Render(uint32_t frame, uint32_t index)
{
	CommandBuffer &buf = cmd_buffers_[frame];
	OnRender(buf, index);
	return buf;
}

std::array vertices = {
	Vertex3D{Fvec2(-0.5f, 0.5f)},
	Vertex3D{Fvec2(0.5f, 0.5f)},
	Vertex3D{Fvec2(0.5f, -0.5f)},
};

VertexBuffer *vb;

void Layer3D::OnInit(Device &device)
{
	vb = new VertexBuffer(device, vertices.size() * sizeof(Vertex3D));
	vb->MapData(vertices.data());

}

void Layer3D::OnClose()
{
	delete vb;
}

void Layer3D::OnRender(CommandBuffer &cb, uint32_t index)
{
	cb.Reset();

	cb.RecordDraw(index, [this](CmdDraw &cmd){
		cmd.BindPipeline(GetPipeline());
		cmd.BindVertexBuffer(*vb);
		auto size = App::Instance()->GetWindow().GetFramebufferSize();
		cmd.SetViewport({0, 0}, size);
		cmd.SetScissor({0, 0}, size);
		cmd.Draw(3, 1);
	});
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

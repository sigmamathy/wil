#include <wil/layer.hpp>
#include <wil/app.hpp>
#include <wil/buffer.hpp>
#include <wil/log.hpp>

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


void Layer3D::OnInit(Device &device)
{
	auto &p = GetPipeline();
	size_t num_set = p.GetDescriptorSetsCount();
	std::vector<uint32_t> ids;
	ids.resize(num_set * App::Instance()->GetFramesInFlight());
	for (uint32_t i = 0; i < ids.size(); ++i)
		ids[i] = i % num_set;

	descriptor_sets_ = GetPipeline().CreateDescriptorSets(ids);
}

void Layer3D::OnClose()
{
	descriptor_sets_.clear();
}

std::unique_ptr<Pipeline> Layer3D::MakePipeline(Device &device)
{
	PipelineCtor ctor;
	ctor.device = &device;
	ctor.shaders[VERTEX_SHADER] = "../shaders/3d.vert.spv";
	ctor.shaders[FRAGMENT_SHADER] = "../shaders/3d.frag.spv";
	ctor.vertex_layout.push_back(wilvrta(0, Vertex3D, pos));
	ctor.vertex_stride = sizeof(Vertex3D);
	auto &layout = ctor.descriptor_set_layouts.emplace_back();
	layout.bindings.emplace_back(UNIFORM_BUFFER, 0, VERTEX_SHADER, sizeof(MVP3D));
	
	return std::make_unique<Pipeline>(ctor);
}

}

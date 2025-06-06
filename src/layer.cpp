#include <wil/layer.hpp>
#include <wil/app.hpp>
#include <wil/buffer.hpp>
#include <wil/log.hpp>

namespace wil {

Layer::Layer(Device &device, const PipelineCtor &ctor)
{
	pipeline_ = std::make_unique<Pipeline>(ctor);
	for (uint32_t i = 0; i < App::Instance()->GetFramesInFlight(); ++i)
		cmd_buffers_.emplace_back(device);
}

Layer::~Layer()
{
}


Layer3D::Layer3D(Device &device) : Layer(device, MakePipeline(device))
{
	auto &p = GetPipeline();
	size_t num_set = p.GetDescriptorSetsCount();
	std::vector<uint32_t> ids;
	ids.resize(num_set * App::Instance()->GetFramesInFlight());
	for (uint32_t i = 0; i < ids.size(); ++i)
		ids[i] = i % num_set;

	descriptor_sets_ = GetPipeline().CreateDescriptorSets(ids);
}

Layer3D::~Layer3D()
{
}

PipelineCtor Layer3D::MakePipeline(Device &device)
{
	PipelineCtor ctor;
	ctor.device = &device;
	ctor.shaders[VERTEX_SHADER] = "../shaders/3d.vert.spv";
	ctor.shaders[FRAGMENT_SHADER] = "../shaders/3d.frag.spv";
	ctor.vertex_layout.push_back(wilvrta(0, Vertex3D, pos));
	ctor.vertex_stride = sizeof(Vertex3D);
	auto &layout = ctor.descriptor_set_layouts.emplace_back();
	layout.bindings.emplace_back(UNIFORM_BUFFER, 0, VERTEX_SHADER, sizeof(MVP3D));
	
	return ctor;
}

}

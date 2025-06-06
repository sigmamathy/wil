#include <wil/layer.hpp>
#include <wil/app.hpp>
#include <wil/buffer.hpp>
#include <wil/log.hpp>

namespace wil {

Layer::Layer(Device &device)
{
	for (uint32_t i = 0; i < App::Instance()->GetFramesInFlight(); ++i)
		cmd_buffers_.emplace_back(device);

	// wil::LogInfo(std::to_string(cmd_buffers_.size()));
}

Layer::~Layer()
{
}


Layer3D::Layer3D(Device &device) : Layer(device)
{
	PipelineCtor ctor;
	ctor.device = &device;
	ctor.shaders[VERTEX_SHADER] = "../shaders/3d.vert.spv";
	ctor.shaders[FRAGMENT_SHADER] = "../shaders/3d.frag.spv";
	ctor.vertex_layout.push_back(wilvrta(0, Vertex3D, pos));
	ctor.vertex_layout.push_back(wilvrta(1, Vertex3D, texcoord));
	ctor.vertex_stride = sizeof(Vertex3D);

	ctor.descriptor_set_layouts.resize(1);
	ctor.descriptor_set_layouts[0].Add(0, UNIFORM_BUFFER, VERTEX_SHADER);
	ctor.descriptor_set_layouts[0].Add(1, COMBINED_IMAGE_SAMPLER, FRAGMENT_SHADER);

	pipeline_ = std::make_unique<Pipeline>(ctor);
	
}

Layer3D::~Layer3D()
{
}

}

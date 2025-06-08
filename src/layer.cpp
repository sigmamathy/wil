#include <wil/layer.hpp>
#include <wil/app.hpp>
#include <wil/buffer.hpp>
#include <wil/log.hpp>

namespace wil {

Layer::Layer(Device &device)
{
	for (uint32_t i = 0; i < App::Instance()->GetFramesInFlight(); ++i)
		cmd_buffers_.emplace_back(device);
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

	ctor.push_constant_stage = VERTEX_SHADER;
	ctor.push_constant_size = sizeof(Fmat4);

	ctor.descriptor_set_layouts.resize(2);
	ctor.descriptor_set_layouts[0].Add(0, UNIFORM_BUFFER, VERTEX_SHADER);
	ctor.descriptor_set_layouts[1].Add(0, COMBINED_IMAGE_SAMPLER, FRAGMENT_SHADER);

	pipeline_ = std::make_unique<Pipeline>(ctor);
	
}

Layer3D::~Layer3D()
{
}

}

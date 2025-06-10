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

	ctor.vertex_shader = "../shaders/3d.vert.spv";
	ctor.fragment_shader = "../shaders/3d.frag.spv";

	ctor.vertex_layout.push_back(wilvrta(0, Vertex3D, pos));
	ctor.vertex_layout.push_back(wilvrta(1, Vertex3D, texcoord));
	ctor.vertex_stride = sizeof(Vertex3D);

	ctor.push_constant_stage = VERTEX_SHADER;
	ctor.push_constant_size = sizeof(Fmat4);

	ctor.descriptor_set_layouts.resize(2);
	ctor.descriptor_set_layouts[0].Add(0, UNIFORM_BUFFER, VERTEX_SHADER);
	ctor.descriptor_set_layouts[1].Add(0, COMBINED_IMAGE_SAMPLER, FRAGMENT_SHADER);

	pipeline_ = std::make_unique<Pipeline>(ctor);
	
	PipelineCtor lct;
	lct.device = &device;

	lct.vertex_shader = "../shaders/light3d.vert.spv";
	lct.fragment_shader = "../shaders/light3d.frag.spv";

	lct.vertex_layout.push_back(wilvrta(0, LightVertex3D, pos));
	lct.vertex_stride = sizeof(LightVertex3D);

	lct.push_constant_stage = VERTEX_SHADER | FRAGMENT_SHADER;
	lct.push_constant_size = sizeof(LightPushConstant3D);

	lct.descriptor_set_layouts.resize(1);
	lct.descriptor_set_layouts[0].Add(0, UNIFORM_BUFFER, VERTEX_SHADER);

	light_pipeline_ = std::make_unique<Pipeline>(lct);
}

Layer3D::~Layer3D()
{
}

}

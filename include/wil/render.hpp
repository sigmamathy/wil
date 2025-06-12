#pragma once

#include "ecs.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "descriptor.hpp"

namespace wil {

struct ModelComponent
{
	std::string path;
};

struct LightComponent
{
	Fvec3 color;
};

class RenderSystem : public System
{
public:

	RenderSystem(Registry& registry, Device &device);

	void Render();

private:

	struct ObjectVertex
	{
		Fvec3 pos;
		Fvec2 texcoord;
		Fvec3 normal;
	};

	struct LightVertex
	{
		Fvec3 pos;
	};

	struct ObjectPushConstant
	{
		Fmat4 model;
	};

	struct LightPushConstant
	{
		Fmat4 model;
		Fvec4 light_color; 
	};

	struct ObjectUniform_0
	{
		alignas(16) Fmat4 view;
		alignas(16) Fmat4 proj;
		alignas(16) Fvec3 view_pos;
	};

	struct LightUniform_0
	{
		alignas(16) Fvec3 pos;
		alignas(16) Fvec3 color;
	};

	void CreatePipelines_(Device &device);

	Registry &registry_;

	EntityView objects_;
	EntityView lights_;

	std::unique_ptr<Pipeline> object_pipeline_;
	std::unique_ptr<Pipeline> light_pipeline_;

	std::unique_ptr<DescriptorPool> object_pool_;
	std::unique_ptr<DescriptorPool> light_pool_;
};

}

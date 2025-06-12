#pragma once

#include "ecs.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "descriptor.hpp"
#include "cmdbuf.hpp"

namespace wil {

struct ModelComponent
{
	std::string path;
	int32_t texture_index;
};

struct LightComponent
{
	Fvec3 color;
};

class RenderSystem : public System
{
public:

	RenderSystem(Registry& registry, Device &device);

	void Render(CommandBuffer &cb, struct FrameData &frame);

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
		alignas(16) Fmat4 model;
	};

	struct LightPushConstant
	{
		alignas(16) Fmat4 model;
		alignas(16) Fvec3 light_color; 
	};

	struct ObjectUniform_0_0
	{
		alignas(16) Fmat4 view;
		alignas(16) Fmat4 proj;
		alignas(16) Fvec3 view_pos;
	};

	struct ObjectUniform_0_1
	{
		alignas(16) Fvec3 pos;
		alignas(16) Fvec3 color;
	};

	struct LightUniform_0_0
	{
		alignas(16) Fmat4 view;
		alignas(16) Fmat4 proj;
	};

	void CreatePipelines_(Device &device);

	void CreateDescriptorSetsAndUniforms_(Device &device);

	Registry &registry_;

	EntityView objects_;
	EntityView lights_;

	std::unique_ptr<Pipeline> object_pipeline_;
	std::unique_ptr<Pipeline> light_pipeline_;

	std::unique_ptr<DescriptorPool> object_pool_;
	std::unique_ptr<DescriptorPool> light_pool_;

	std::vector<DescriptorSet> object_0_sets;
	std::vector<DescriptorSet> object_1_sets;
	std::vector<DescriptorSet> light_0_sets;

	std::vector<UniformBuffer> object_0_0_uniforms; // GlobalData
	std::vector<UniformBuffer> object_0_1_uniforms; // Lights
	std::vector<UniformBuffer> light_0_0_uniforms; // GlobalData
};

}

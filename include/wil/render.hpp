#pragma once

#include "ecs.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "descriptor.hpp"
#include "cmdbuf.hpp"
#include "model.hpp"

namespace wil {

enum ShapeType
{
	CUBE,
	SPHERE,
	CYLINDER,
	PYRAMID,
	CONE,
	ARROW,
};

struct Shape
{
	ShapeType shape;
	Fvec3 color;
};

struct ModelComponent
{
	std::string path;
	uint32_t texture_index;
};

struct PointLightComponent
{
	Fvec3 color;
	float linear;
	float quadratic;
};

struct SpotLightComponent
{
	Fvec3 color;
	Fvec3 dir;
	float cutoff;
	float linear;
	float quadratic;
};

struct Camera
{
	Fvec3 position;
	float h_angle;
	float v_angle;

	void MoveStraight(float val);
	void MoveStraightNoUp(float val);

	void MoveSideway(float val);
	void MoveUp(float val);
};

class RenderSystem : public System
{
public:

	RenderSystem(Registry& registry, Device &device);

	void Render(CommandBuffer &cb, struct FrameData &frame);

	Camera &GetCamera() { return camera_; }

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
		WIL_ALIGN_STD140(Fmat4) model;
	};

	struct LightPushConstant
	{
		WIL_ALIGN_STD140(Fmat4) model;
		WIL_ALIGN_STD140(Fvec3) light_color; 
	};

	struct ObjectUniform_0_0
	{
		WIL_ALIGN_STD140(Fmat4) view;
		WIL_ALIGN_STD140(Fmat4) proj;
		WIL_ALIGN_STD140(Fvec3) view_pos;
	};

	struct ObjectDirectionalLight
	{
		WIL_ALIGN_STD140(Fvec3) dir;
		WIL_ALIGN_STD140(Fvec3) color;
	};

	struct ObjectPointLight
	{
		WIL_ALIGN_STD140(Fvec3) pos;
		WIL_ALIGN_STD140(Fvec3) color;
		WIL_ALIGN_STD140(float) linear;
		WIL_ALIGN_STD140(float) quadratic;
	};

	struct ObjectSpotLight
	{
		WIL_ALIGN_STD140(Fvec3) pos;
		WIL_ALIGN_STD140(Fvec3) dir;
		WIL_ALIGN_STD140(Fvec3) color;
		WIL_ALIGN_STD140(float) cutoff;
		WIL_ALIGN_STD140(float) linear;
		WIL_ALIGN_STD140(float) quadratic;
	};

	struct ObjectStorage_0_1
	{
		static constexpr unsigned MAX_COUNT = 100;

		ObjectDirectionalLight directional;
		ObjectPointLight points[MAX_COUNT];
		ObjectSpotLight spots[MAX_COUNT];
		WIL_ALIGN_STD140(unsigned) pl_count;
		WIL_ALIGN_STD140(unsigned) sl_count;
	};

	struct LightUniform_0_0
	{
		WIL_ALIGN_STD140(Fmat4) view;
		WIL_ALIGN_STD140(Fmat4) proj;
	};

	void CreatePipelines_(Device &device);

	void CreateDescriptorSetsAndUniforms_(Device &device);

	Registry &registry_;
	Device &device_;

	EntityView objects_;
	EntityView point_lights_, spot_lights_;

	Camera camera_;

	std::unique_ptr<Pipeline> object_pipeline_;
	std::unique_ptr<Pipeline> light_pipeline_;

	std::unique_ptr<DescriptorPool> object_pool_;
	std::unique_ptr<DescriptorPool> light_pool_;

	std::vector<DescriptorSet> object_0_sets;
	std::vector<DescriptorSet> object_1_sets;
	std::vector<DescriptorSet> light_0_sets;

	std::vector<UniformBuffer> object_0_0_uniforms; // GlobalData
	std::vector<StorageBuffer> object_0_1_storages; // Lights
	std::vector<UniformBuffer> light_0_0_uniforms; // GlobalData

	std::unordered_map<std::string, Model> models_;

	VertexBuffer cube_vbo;
	IndexBuffer cube_ibo;
};

}

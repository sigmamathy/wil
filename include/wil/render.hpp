#pragma once

#include "ecs.hpp"
#include "pipeline.hpp"

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

	RenderSystem(Registry& registry);

	void Render();

private:

	Registry &registry_;

	EntityView objects_;
	EntityView lights_;

	std::unique_ptr<Pipeline> object_pipeline_;
};

}

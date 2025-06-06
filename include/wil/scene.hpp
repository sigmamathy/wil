#pragma once

#include "core.hpp"
#include "layer.hpp"
#include "drawsync.hpp"
#include <vector>

namespace wil {

class Scene
{
public:

	Scene(Device &device, const std::vector<std::string> &layers);

	virtual ~Scene();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Scene);

	bool Render(uint32_t frame);
	
	virtual std::string GetName() const = 0;

private:

	std::vector<Layer*> layers_;
	std::vector<DrawPresentSynchronizer*> syncs_;

};

}

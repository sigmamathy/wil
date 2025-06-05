#pragma once

#include "core.hpp"
#include "layer.hpp"
#include "drawsync.hpp"
#include <vector>

namespace wil {

class Scene
{
public:

	Scene() = default;

	virtual ~Scene() = default;

	WIL_DELETE_COPY_AND_REASSIGNMENT(Scene);

	void Init(Device &device);

	void Free();

	void Render(uint32_t frame);
	
	virtual void OnInit(Device &device) {}

	virtual std::vector<std::string> SetupLayers() const = 0;

	virtual std::string GetName() const = 0;

private:

	std::vector<Layer*> layers_;
	std::vector<DrawPresentSynchronizer*> syncs_;

};

}

#pragma once

#include "core.hpp"
#include "drawsync.hpp"
#include <vector>

namespace wil {

class Scene
{
public:

	Scene(Device &device);

	virtual ~Scene();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Scene);

	DrawPresentSynchronizer &GetDrawPresentSynchronizer(uint32_t frame_index) {
		return *syncs_[frame_index];
	}

	virtual bool Update(struct FrameData &frame) = 0;
	
	virtual std::string GetName() const = 0;

private:

	std::vector<DrawPresentSynchronizer*> syncs_;

};

}

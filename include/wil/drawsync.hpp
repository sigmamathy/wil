#pragma once

#include "core.hpp"
#include "device.hpp"
#include "cmdbuf.hpp"
#include <vector>

namespace wil {

class DrawPresentSynchronizer
{
public:

	DrawPresentSynchronizer(Device &device, uint32_t num_of_render);

	~DrawPresentSynchronizer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(DrawPresentSynchronizer);

	bool AcquireImageIndex(uint32_t *index);

	void SubmitDraw(const std::vector<CommandBuffer*> &buffers);

	bool PresentToScreen(uint32_t image_index);

private:
	Device &device_;
	VendorPtr image_available_semaphore_;
	std::vector<VendorPtr> render_semaphores_;
	VendorPtr in_flight_fence_;
};

}

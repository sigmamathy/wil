#pragma once

#include "device.hpp"

namespace wil {

class VertexBuffer
{
public:

    VertexBuffer(Device &device, size_t size);

    ~VertexBuffer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(VertexBuffer);

    void MapData(const void* src);

	VendorPtr GetVkBufferPtr_() const { return buffer_ptr_; }

    size_t GetSize() const { return size_; }

private:

	Device &device_;

    VendorPtr buffer_ptr_;
    VendorPtr memory_ptr_;
    size_t size_;
};

}

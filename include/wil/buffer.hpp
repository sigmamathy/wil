#pragma once

#include "device.hpp"

namespace wil {

class VertexBuffer
{
public:

	VertexBuffer() : buffer_ptr_(nullptr) {}

    VertexBuffer(Device &device, size_t size);

    ~VertexBuffer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(VertexBuffer);

	VertexBuffer(VertexBuffer &&buffer);

	VertexBuffer& operator=(VertexBuffer &&buffer);

    void MapData(const void* src);

	VendorPtr GetVkBufferPtr_() const { return buffer_ptr_; }

    size_t GetSize() const { return size_; }

private:

	Device *device_;

    VendorPtr buffer_ptr_;
    VendorPtr memory_ptr_;
    size_t size_;
};

class IndexBuffer
{
public:

	IndexBuffer() : buffer_ptr_(nullptr) {}

    IndexBuffer(Device &device, size_t size);

    ~IndexBuffer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(IndexBuffer);

	IndexBuffer(IndexBuffer &&buffer);

	IndexBuffer& operator=(IndexBuffer &&buffer);

    void MapData(const unsigned* src);

	VendorPtr GetVkBufferPtr_() const { return buffer_ptr_; }

    size_t GetSize() const { return size_; }

private:

	Device *device_;

    VendorPtr buffer_ptr_;
    VendorPtr memory_ptr_;
    size_t size_;
};

class UniformBuffer
{
public:

	UniformBuffer() : buffer_ptr_(nullptr) {}

    UniformBuffer(Device &device, size_t size);

    ~UniformBuffer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(UniformBuffer);

	UniformBuffer(UniformBuffer &&buffer);

    void Update(const void* src);

	VendorPtr GetVkBufferPtr_() const { return buffer_ptr_; }

    size_t GetSize() const { return size_; }

private:

	Device *device_;

    VendorPtr buffer_ptr_;
    VendorPtr memory_ptr_;
    size_t size_;
	void *data_;
};

class Texture
{
public:

	Texture() : image_ptr_(nullptr) {}

	Texture(Device &dev, const std::string &path);

	Texture(Device &dev, const void *data, size_t size, uint32_t width, uint32_t height);

	~Texture();

	WIL_DELETE_COPY_AND_REASSIGNMENT(Texture);

	Texture(Texture &&tex);

	Texture &operator=(Texture &&tex);

	VendorPtr GetVkImageViewPtr_() const { return image_view_ptr_; }

	VendorPtr GetVkSamplerPtr_() const { return sampler_ptr_; }

private:

	void Init_(Device &dev, const void *data, size_t size, uint32_t width, uint32_t height);

	Device *device_;

	VendorPtr image_ptr_;
	VendorPtr memory_ptr_;
	VendorPtr image_view_ptr_;
	VendorPtr sampler_ptr_;
};

class DepthBuffer
{
public:

	DepthBuffer(Device &dev);

	~DepthBuffer();

	WIL_DELETE_COPY_AND_REASSIGNMENT(DepthBuffer);

	uint32_t GetFormat() const { return format_; }

	VendorPtr GetVkImageViewPtr_() const { return image_view_ptr_; }

private:

	Device &device_;

	VendorPtr image_ptr_;
	VendorPtr memory_ptr_;
	VendorPtr image_view_ptr_;

	uint32_t format_;
};

}

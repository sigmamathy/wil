#pragma once

#include "buffer.hpp"
#include <memory>

namespace wil {

struct Mesh
{
	std::unique_ptr<VertexBuffer> vertex_buffer;
	std::unique_ptr<IndexBuffer> index_buffer;
	int material_index;
};

// Currently only support .gltf/.glb files
class Model
{
public:

	using VertexHandler = std::function<void(void *output, Fvec3 position, Fvec2 texcoord)>;

	Model(Device &device, const std::string &path, size_t vertex_size, const VertexHandler &fn);

private:
	std::vector<Mesh> meshes_;
	std::vector<std::unique_ptr<Texture>> textures_;
};

}

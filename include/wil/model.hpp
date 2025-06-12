#pragma once

#include "buffer.hpp"
#include <cstring>
#include <optional>

namespace wil {

struct Mesh
{
	VertexBuffer vertex_buffer;
	std::optional<IndexBuffer> index_buffer;
	int material_index;
	uint32_t draw_count;
};

// Currently only support .gltf/.glb files
class Model
{
public:

	using VertexHandler = std::function<void(void *output, Fvec3 position, Fvec2 texcoord, Fvec3 normal)>;

	Model(Device &device, const std::string &path, size_t vertex_size, const VertexHandler &fn);

	const std::vector<Mesh> &GetMeshes() const { return meshes_; }
	const std::vector<Texture> &GetTextures() const { return textures_; }

	size_t GetTextureCount() const { return textures_.size(); }

private:
	std::vector<Mesh> meshes_;
	std::vector<Texture> textures_;
};

}

#include <wil/model.hpp>
#include <wil/log.hpp>

#include <filesystem>
#include <tinygltf/tiny_gltf.h>

namespace wil {

static tinygltf::Model LoadGLTFModel_(const std::string& filename)
{
	namespace fs = std::filesystem;
	std::string ext = fs::path(filename).extension();
	bool is_binary;
	if (ext == ".glb") is_binary = true;
	else if (ext == ".gltf") is_binary = false;
	else WIL_LOGERROR("Unsupported file type {} detected in {}", ext, filename);

    tinygltf::TinyGLTF loader;
	tinygltf::Model model;

    std::string err, warn;
    bool status = is_binary
		? loader.LoadBinaryFromFile(&model, &err, &warn, filename)
		: loader.LoadASCIIFromFile(&model, &err, &warn, filename);

    if (!warn.empty())
		WIL_LOGWARN("Warning from tinygltf: {}", warn);

    if (!err.empty()) 
		WIL_LOGERROR("Error from tinygltf: {}", warn);

    if (!status)
		WIL_LOGERROR("Unable to load model {}", filename);
    
    return model;
}

static std::vector<Mesh>
ExtractMeshes_(const tinygltf::Model& model, Device &device, size_t vsize, const Model::VertexHandler &handler)
{
	std::vector<Mesh> result;
	result.reserve(model.meshes.size());

    for (const auto& mesh : model.meshes)
	{
        for (const auto& primitive : mesh.primitives)
		{
            Mesh& m = result.emplace_back();
            m.material_index = primitive.material; // Store material index

            // Extract vertices and indices (similar to ExtractVertexData, but per primitive)
            const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
            const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
            const auto& posBuffer = model.buffers[posBufferView.buffer];

            const auto& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
            const auto& normalBufferView = model.bufferViews[posAccessor.bufferView];
            const auto& normalBuffer = model.buffers[posBufferView.buffer];

            const auto& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
            const auto& texBufferView = model.bufferViews[texAccessor.bufferView];
            const auto& texBuffer = model.buffers[texBufferView.buffer];

            size_t posOffset = posBufferView.byteOffset + posAccessor.byteOffset;
			size_t normalOffset = normalBufferView.byteOffset + normalAccessor.byteOffset;
            size_t texOffset = texBufferView.byteOffset + texAccessor.byteOffset;
            size_t vertexCount = posAccessor.count;

			std::vector<char> vertices_data;
			vertices_data.resize(vsize * vertexCount);

            for (size_t i = 0; i < vertexCount; ++i)
			{
                const float* posData = reinterpret_cast<const float*>(
						&posBuffer.data[posOffset + i * posBufferView.byteStride]);
                const float* normalData = reinterpret_cast<const float*>(
						&normalBuffer.data[normalOffset + i * normalBufferView.byteStride]);
                const float* texData = reinterpret_cast<const float*>(
						&texBuffer.data[texOffset + i * texBufferView.byteStride]);

				Fvec3 pos = {posData[0], posData[1], posData[2]};
				Fvec3 normal = {normalData[0], normalData[1], normalData[2]};
				Fvec2 texcoord = {texData[0], texData[1]};
				handler(vertices_data.data() + i * vsize, pos, texcoord, normal);
            }

			m.vertex_buffer = std::make_unique<VertexBuffer>(device, vsize * vertexCount);
			m.vertex_buffer->MapData(vertices_data.data());
			m.draw_count = vertexCount;

            if (primitive.indices >= 0) {
                const auto& indexAccessor = model.accessors[primitive.indices];
                const auto& indexBufferView = model.bufferViews[indexAccessor.bufferView];
                const auto& indexBuffer = model.buffers[indexBufferView.buffer];
                size_t indexOffset = indexBufferView.byteOffset + indexAccessor.byteOffset;
                size_t indexCount = indexAccessor.count;

				std::vector<unsigned> indices;
				indices.reserve(indexCount);

                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* indexData = reinterpret_cast<const uint16_t*>(&indexBuffer.data[indexOffset]);
                    for (size_t i = 0; i < indexCount; ++i) {
                        indices.push_back(static_cast<unsigned int>(indexData[i]));
                    }
                } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* indexData = reinterpret_cast<const uint32_t*>(&indexBuffer.data[indexOffset]);
                    for (size_t i = 0; i < indexCount; ++i) {
                        indices.push_back(indexData[i]);
                    }
                }

				m.index_buffer = std::make_unique<IndexBuffer>(device, sizeof(uint32_t) * indexCount);
				m.index_buffer->MapData(indices.data());
				m.draw_count = indexCount;
            }
        }
    }

	return result;
}

static std::vector<std::unique_ptr<Texture>>
LoadTextures_(const tinygltf::Model& model, Device &device)
{
	std::vector<std::unique_ptr<Texture>> textures;

    for (const auto& material : model.materials)
	{
        // Check for base color texture (common in PBR materials)
        if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
		{
            const auto& tex = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
            const auto& image = model.images[tex.source];

            // Load image data
            if (!image.image.empty()) {
				textures.push_back(std::make_unique<Texture>(
							device, image.image.data(), image.width * image.height * 4, image.width, image.height));
            } else {
				WIL_LOGERROR("Texture image data is empty");
            }
        }
    }

	return textures;
}

Model::Model(Device &device, const std::string &path, size_t vertex_size, const VertexHandler &fn)
{
	tinygltf::Model model = LoadGLTFModel_(path);
	meshes_ = ExtractMeshes_(model, device, vertex_size, fn);
	textures_ = LoadTextures_(model, device);
}

}

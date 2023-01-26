#ifndef VE_MESH_HPP
#define VE_MESH_HPP

#include "ve_buffer.hpp"
#include "ass.hpp"
#include "glms.hpp"

struct VeMeshVertex
{
    glm::vec3 positon_{};
    glm::vec3 normal_{};
    glm::vec3 uv_{};
    glm::vec3 color_{1.0f, 0.0f, 0.0f};
};

template <uint32_t MAX_INSTANCES>
class VeMesh
{
  private:
    VeBufferBase VBO_{};
    VeBufferBase VBO_INSTANCE_{};
    std::vector<VeMeshVertex> vertices_{};
    std::vector<size_t> vbo_offsets_{};

    VeBufferBase IBO_{};
    std::vector<uint32_t> indices_{};
    std::vector<uint32_t> indices_count_{};
    std::vector<size_t> ibo_offsets_{};

    std::array<glm::mat4, MAX_INSTANCES> instances_{};

  public:
    VeMesh(const std::string file_path);

    void draw();

    static std::array<VkVertexInputBindingDescription, 2> get_bindings();
    static std::array<VkVertexInputAttributeDescription, 5> get_attributes();
};

template <uint32_t MAX_INSTANCES>
inline VeMesh<MAX_INSTANCES>::VeMesh(const std::string file_path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_GenNormals);

    if (scene == nullptr)
    {
        std::cout << "Do not load file " << file_path << std::endl;
        return;
    }

    size_t vertex_offset = 0;
    size_t ibo_offset = 0;
    for (uint32_t i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i];

        for (size_t v = 0; v < mesh->mNumVertices; v++)
        {
            VeMeshVertex vertex{};
            vertex.positon_ = glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
            vertex.normal_ = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);

            vertices_.push_back(vertex);
        }

        if (mesh->mTextureCoords[0] != nullptr)
        {
            for (size_t v = vertex_offset; v < vertex_offset + mesh->mNumVertices; v++)
            {
                vertices_[v].uv_ = glm::vec3(mesh->mTextureCoords[0][v - vertex_offset].x, //
                                             mesh->mTextureCoords[0][v - vertex_offset].y, //
                                             mesh->mTextureCoords[0][v - vertex_offset].z);
            }
        }

        if (mesh->mColors[0] != nullptr)
        {
            for (size_t v = vertex_offset; v < vertex_offset + mesh->mNumVertices; v++)
            {
                vertices_[v].color_ = glm::vec3(mesh->mColors[0][v - vertex_offset].r, //
                                                mesh->mColors[0][v - vertex_offset].g, //
                                                mesh->mColors[0][v - vertex_offset].b);
            }
        }

        vbo_offsets_.push_back(vertex_offset);
        vertex_offset += mesh->mNumVertices;

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            indices_.push_back(mesh->mFaces[i].mIndices[0]);
            indices_.push_back(mesh->mFaces[i].mIndices[1]);
            indices_.push_back(mesh->mFaces[i].mIndices[2]);
        }
        ibo_offsets_.push_back(ibo_offset);
        indices_count_.push_back(3 * mesh->mNumFaces);
        ibo_offset += 3 * mesh->mNumFaces;
    }
}

template <uint32_t MAX_INSTANCES>
inline void VeMesh<MAX_INSTANCES>::draw()
{
}

template <uint32_t MAX_INSTANCES>
inline std::array<VkVertexInputBindingDescription, 2> VeMesh<MAX_INSTANCES>::get_bindings()
{
    std::array<VkVertexInputBindingDescription, 2> binding{};

    binding[0].binding = 0;
    binding[0].stride = sizeof(VeMeshVertex);
    binding[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    binding[1].binding = 1;
    binding[1].stride = sizeof(glm::mat4);
    binding[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return binding;
}

template <uint32_t MAX_INSTANCES>
inline std::array<VkVertexInputAttributeDescription, 5> VeMesh<MAX_INSTANCES>::get_attributes()
{
    std::array<VkVertexInputAttributeDescription, 5> attributes{};
    for (uint32_t i = 0; i < 4; i++)
    {
        attributes[i].binding = 0;
        attributes[i].location = i;
        attributes[i].format = VK_FORMAT_R32G32B32_SFLOAT;
    }

    attributes[0].offset = offsetof(VeMeshVertex, positon_);
    attributes[1].offset = offsetof(VeMeshVertex, normal_);
    attributes[2].offset = offsetof(VeMeshVertex, uv_);
    attributes[3].offset = offsetof(VeMeshVertex, color_);

    for (uint32_t i = 4; i < 8; i++)
    {
        attributes[i].binding = 1;
        attributes[i].location = i;
        attributes[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributes[i].offset = (i - 4) * sizeof(glm::vec4);
    }

    return attributes;
}

#endif // VE_MESH_HPP

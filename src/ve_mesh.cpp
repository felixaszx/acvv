#include "ve_mesh.hpp"

VeMesh::VeMesh(const std::string file_path, uint32_t max_instance)
    : MAX_INSTANCE(max_instance)
{
    instances_.resize(max_instance);
    std::fill(instances_.begin(), instances_.end(), glm::mat4(1.0f));

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_GenNormals);

    if (scene == nullptr)
    {
        std::cout << "Do not load file " << file_path << std::endl;
        return;
    }

    size_t vertex_offset = 0;
    size_t indices_buffer_offset = 0;
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

        vert_buffer_offsets_.push_back(vertex_offset);
        vertex_offset += mesh->mNumVertices;

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            indices_.push_back(mesh->mFaces[i].mIndices[0]);
            indices_.push_back(mesh->mFaces[i].mIndices[1]);
            indices_.push_back(mesh->mFaces[i].mIndices[2]);
        }

        indices_buffer_offsets_.push_back(indices_buffer_offset);
        indices_count_.push_back(3 * mesh->mNumFaces);
        indices_buffer_offset += 3 * mesh->mNumFaces;
    }
}

void VeMesh::create(VeDeviceLayer device_layer)
{
    // load vertex
    VkBufferCreateInfo vert_buffer_info{};
    vert_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vert_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vert_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vert_buffer_info.size = vertices_.size() * sizeof(VeMeshVertex);
    VmaAllocationCreateInfo vert_alloc_info{};
    vert_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateBuffer(device_layer, &vert_buffer_info, &vert_alloc_info, &vert_buffer_, &vert_buffer_, nullptr);

    VkBufferCreateInfo index_buffer_info{};
    index_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    index_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    index_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    index_buffer_info.size = indices_.size() * sizeof(uint32_t);
    VmaAllocationCreateInfo index_alloc_info{};
    index_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    vmaCreateBuffer(device_layer, &index_buffer_info, &index_alloc_info, &index_buffer_, &index_buffer_, nullptr);

    VkBufferCreateInfo instance_buffer_info{};
    instance_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    instance_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    instance_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    instance_buffer_info.size = indices_.size() * sizeof(uint32_t);
    VmaAllocationCreateInfo instance_alloc_info{};
    instance_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    instance_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaCreateBuffer(device_layer, &instance_buffer_info, &instance_alloc_info, &instance_buffer_, &instance_buffer_,
                    nullptr);
    vmaMapMemory(device_layer, instance_buffer_, &instance_mapping_);
    if (instance_mapping_ == nullptr)
    {
        std::cout << "Do not map instance buffer" << std::endl;
    }

    void* staging_mapping = nullptr;
    VeBufferBase staging_buffer{};
    VkBufferCreateInfo staging_buffer_info{};
    staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    staging_buffer_info.size = vertices_.size() * sizeof(VeMeshVertex);
    VmaAllocationCreateInfo staging_alloc_info{};
    staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaCreateBuffer(device_layer, &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_buffer, nullptr);
    vmaMapMemory(device_layer, staging_buffer, &staging_mapping);
    if (staging_mapping == nullptr)
    {
        std::cout << "vertex staging create fail" << std::endl;
    }
    memcpy(staging_mapping, vertices_.data(), vertices_.size() * sizeof(VeMeshVertex));
    vmaUnmapMemory(device_layer, staging_buffer);

    VeSingleTimeCmdBase cmd;
    cmd.begin(device_layer);
    VkBufferCopy copy_region{};
    copy_region.size = vertices_.size() * sizeof(VeMeshVertex);
    vkCmdCopyBuffer(cmd, staging_buffer, vert_buffer_, 1, &copy_region);
    cmd.end(device_layer);
    vmaDestroyBuffer(device_layer, staging_buffer, staging_buffer);

    // load indices
    staging_mapping = nullptr;
    staging_buffer_info.size = indices_.size() * sizeof(uint32_t);
    vmaCreateBuffer(device_layer, &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_buffer, nullptr);
    vmaMapMemory(device_layer, staging_buffer, &staging_mapping);
    if (staging_mapping == nullptr)
    {
        std::cout << "indices staging create fail" << std::endl;
    }
    memcpy(staging_mapping, indices_.data(), indices_.size() * sizeof(uint32_t));
    vmaUnmapMemory(device_layer, staging_buffer);

    cmd.begin(device_layer);
    copy_region.size = indices_.size() * sizeof(uint32_t);
    vkCmdCopyBuffer(cmd, staging_buffer, index_buffer_, 1, &copy_region);
    cmd.end(device_layer);
    vmaDestroyBuffer(device_layer, staging_buffer, staging_buffer);
}

void VeMesh::draw(VkCommandBuffer cmd)
{
    for (size_t i = 0; i < vert_buffer_offsets_.size(); i++)
    {
        VkBuffer vertex_buffers[2] = {vert_buffer_, instance_buffer_};
        VkDeviceSize vert_offsets[2] = {vert_buffer_offsets_[i] * sizeof(VeMeshVertex), 0};
        vkCmdBindVertexBuffers(cmd, 0, 2, vertex_buffers, vert_offsets);
        vkCmdBindIndexBuffer(cmd, index_buffer_, indices_buffer_offsets_[i] * sizeof(uint32_t), VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, indices_count_[i], update_size, 0, 0, 0);
    }
}

void VeMesh::update()
{
    update_size = current_instance < MAX_INSTANCE ? current_instance : MAX_INSTANCE;
    memcpy(instance_mapping_, instances_.data(), update_size * sizeof(glm::mat4));
}

void VeMesh::destroy(VeDeviceLayer device_layer)
{
    vmaUnmapMemory(device_layer, instance_buffer_);
    vmaDestroyBuffer(device_layer, instance_buffer_, instance_buffer_);
    vmaDestroyBuffer(device_layer, vert_buffer_, vert_buffer_);
    vmaDestroyBuffer(device_layer, index_buffer_, index_buffer_);
}

std::array<VkVertexInputBindingDescription, 2> VeMesh::get_bindings()
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

std::array<VkVertexInputAttributeDescription, 8> VeMesh::get_attributes()
{
    std::array<VkVertexInputAttributeDescription, 8> attributes{};
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

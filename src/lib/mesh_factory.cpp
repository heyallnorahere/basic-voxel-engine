#include "bve_pch.h"
#include "mesh_factory.h"
#include "registry.h"
#include "block.h"
namespace bve {
    struct vertex {
        glm::vec3 position, normal;
        glm::vec2 uv;
    };
    struct block_data {
        GLint texture;
        glm::mat4 model;
    };
    struct full_vertex {
        vertex vertex_data;
        block_data block_data;
    };
    struct cluster_member {
        std::vector<glm::ivec3> surroundings;
        uint32_t cluster_index;
    };
    using cluster_member_map = std::unordered_map<glm::ivec3, cluster_member, hash_vector<3, int32_t>>;
    static void walk(cluster_member_map& map, glm::ivec3 position, uint32_t& cluster_count) {
        cluster_member& member = map[position];
        if (member.cluster_index != (uint32_t)-1) {
            return;
        }
        for (const auto& offset : member.surroundings) {
            glm::ivec3 neighbor_position = position + offset;
            const auto& neighbor = map[neighbor_position];
            if (neighbor.cluster_index != (uint32_t)-1) {
                member.cluster_index = neighbor.cluster_index;
                break;
            }
        }
        if (member.cluster_index == (uint32_t)-1) {
            member.cluster_index = cluster_count++;
        }
        for (const auto& offset : member.surroundings) {
            walk(map, position + offset, cluster_count);
        }
    }
    mesh_factory::mesh_factory(std::shared_ptr<world> _world) {
        this->m_world = _world;
    }
    std::vector<std::vector<mesh_factory::processed_voxel>> mesh_factory::get_clusters() {
        auto& block_register = registry::get().get_register<block>();
        std::vector<glm::ivec3> offsets = {
            glm::ivec3(-1, 0, 0), glm::ivec3(1, 0, 0),
            glm::ivec3(0, -1, 0), glm::ivec3(0, 1, 0),
            glm::ivec3(0, 0, -1), glm::ivec3(0, 0, 1)
        };
        std::unordered_map<glm::ivec3, cluster_member, hash_vector<3, int32_t>> cluster_members;
        glm::ivec3 size = this->m_world->get_size();
        for (int32_t x = 0; x < size.x; x++) {
            for (int32_t y = 0; y < size.y; y++) {
                for (int32_t z = 0; z < size.z; z++) {
                    glm::ivec3 position = glm::ivec3(x, y, z);
                    size_t block_id;
                    this->m_world->get_block(position, block_id);
                    if (block_id == 0) {
                        continue;
                    }
                    cluster_member member;
                    for (const auto& offset : offsets) {
                        glm::ivec3 block_position = position + offset;
                        if (block_position.x < 0 || block_position.x >= size.x ||
                            block_position.y < 0 || block_position.y >= size.y ||
                            block_position.z < 0 || block_position.z >= size.z) {
                            continue;
                        }
                        this->m_world->get_block(block_position, block_id);
                        if (block_id == 0) {
                            continue;
                        }
                        member.surroundings.push_back(offset);
                    }
                    member.cluster_index = (uint32_t)-1;
                    cluster_members.insert({ position, member });
                }
            }
        }
        uint32_t cluster_count = 0;
        for (const std::pair<glm::ivec3, cluster_member>& pair : cluster_members) {
            walk(cluster_members, pair.first, cluster_count);
        }
        std::vector<std::vector<processed_voxel>> clusters(cluster_count);
        // after that iteration, iterate the map again to get the clusters
        for (const std::pair<glm::ivec3, cluster_member>& pair : cluster_members) {
            clusters[pair.second.cluster_index].push_back({ pair.first, pair.second.surroundings });
        }
        return clusters;
    }
    void mesh_factory::create_mesh(std::vector<processed_voxel> voxels, GLuint& vertex_buffer, GLuint& index_buffer, size_t& index_count) {
        

        glGenBuffers(1, &vertex_buffer);
        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    }
    std::vector<vertex_attribute> mesh_factory::get_vertex_attributes() {
        size_t stride = sizeof(full_vertex);
        return {
            { stride, offsetof(vertex, position), vertex_attribute_type::VEC3, false },
            { stride, offsetof(vertex, normal), vertex_attribute_type::VEC3, false },
            { stride, offsetof(vertex, uv), vertex_attribute_type::VEC2, false },
            { stride, offsetof(block_data, texture) + sizeof(vertex), vertex_attribute_type::INT, false },
            { stride, offsetof(block_data, model) + sizeof(vertex), vertex_attribute_type::MAT4, false }
        };
    }
}
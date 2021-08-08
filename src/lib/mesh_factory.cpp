#include "bve_pch.h"
#include "mesh_factory.h"
namespace bve {
    struct vertex {
        glm::vec3 position, normal;
        glm::vec2 uv;
    };
    struct block_data {
        GLint texture;
        glm::mat4 model;
    };
    mesh_factory::mesh_factory(std::shared_ptr<world> _world) {
        this->m_world = _world;
    }
    std::vector<std::vector<glm::ivec3>> mesh_factory::get_clusters() {
        std::vector<std::vector<glm::ivec3>> clusters;
        // todo: isolate clusters of voxels and add them to the list
        return clusters;
    }
    void mesh_factory::create_mesh(std::vector<glm::ivec3> voxels, GLuint& vertex_buffer, GLuint& index_buffer, size_t& index_count) {
        // todo: assemble mesh of all of the voxels in the list
    }
    std::vector<vertex_attribute> mesh_factory::get_vertex_attributes() {
        size_t stride = sizeof(vertex) + sizeof(block_data);
        return {
            { stride, offsetof(vertex, position), vertex_attribute_type::VEC3, false },
            { stride, offsetof(vertex, normal), vertex_attribute_type::VEC3, false },
            { stride, offsetof(vertex, uv), vertex_attribute_type::VEC2, false },
            { stride, offsetof(block_data, texture) + sizeof(vertex), vertex_attribute_type::INT, false },
            { stride, offsetof(block_data, model) + sizeof(vertex), vertex_attribute_type::MAT4, false }
        };
    }
}
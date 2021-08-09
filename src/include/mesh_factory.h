#pragma once
#include "world.h"
#include "renderer.h"
namespace bve {
    struct command_list;
    class mesh_factory {
    public:
        struct processed_voxel {
            glm::ivec3 position;
            std::vector<glm::ivec3> surroundings;
        };
        mesh_factory(std::shared_ptr<world> _world);
        std::vector<std::vector<processed_voxel>> get_clusters();
        void create_mesh(std::vector<processed_voxel> voxels, GLuint& vertex_buffer, GLuint& index_buffer, size_t& index_count);
        std::vector<vertex_attribute> get_vertex_attributes();
    private:
        std::shared_ptr<world> m_world;
    };
}
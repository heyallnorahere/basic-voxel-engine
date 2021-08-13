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
        mesh_factory(ref<world> _world);
        std::vector<std::vector<processed_voxel>> get_clusters();
        ref<mesh> create_mesh(std::vector<processed_voxel> voxels);
        std::vector<vertex_attribute> get_vertex_attributes();
    private:
        ref<world> m_world;
    };
}
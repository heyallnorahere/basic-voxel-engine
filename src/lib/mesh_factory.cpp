#include "bve_pch.h"
#include "mesh_factory.h"
namespace bve {
    struct command_list {
        GLuint vertex_array_object;
        std::vector<std::tuple<GLuint, GLuint>> meshes;
    };
    mesh_factory::mesh_factory(std::shared_ptr<world> _world) {
        this->m_world = _world;
    }
    std::vector<std::vector<glm::ivec3>> mesh_factory::get_clusters() {
        std::vector<std::vector<glm::ivec3>> clusters;
        // todo: isolate clusters of voxels and add them to the list
        return clusters;
    }
    void mesh_factory::create_mesh(std::vector<glm::ivec3> voxels, GLuint& vertex_buffer, GLuint& index_buffer) {
        // todo: assemble mesh of all of the voxels in the list
    }
}
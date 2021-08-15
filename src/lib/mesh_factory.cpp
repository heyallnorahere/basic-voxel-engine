#include "bve_pch.h"
#include "mesh_factory.h"
#include "registry.h"
#include "block.h"
namespace bve {
    struct vertex {
        glm::vec3 position, normal;
        glm::vec2 uv;
        int32_t block_id;
    };
    struct cluster_member {
        std::vector<glm::ivec3> surroundings;
        uint32_t cluster_index;
    };
    struct face {
        std::vector<vertex> vertices;
        glm::vec3 normal;
    };
    class mesh_factory_mesh : public mesh {
    public:
        mesh_factory_mesh(const std::vector<vertex>& vertices, const std::vector<uint32_t>& indices) {
            this->m_vertices = vertices;
            this->m_indices = indices;
        }
        virtual const void* vertex_buffer_data() const override {
            return this->m_vertices.data();
        }
        virtual size_t vertex_buffer_data_size() const override {
            return this->m_vertices.size() * sizeof(vertex);
        }
        virtual size_t vertex_count() const override {
            return this->m_vertices.size();
        }
        virtual std::vector<uint32_t> index_buffer_data() const override {
            return this->m_indices;
        }
    private:
        std::vector<vertex> m_vertices;
        std::vector<uint32_t> m_indices;
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
    template<typename T> static bool equal(const T& v1, const T& v2) {
        return v1 == v2;
    }
    template<typename T> static bool contains(const std::vector<T>& container, const T& search_for, std::function<bool(const T&, const T&)> comparer = equal<T>) {
        for (const auto& element : container) {
            if (comparer(element, search_for)) {
                return true;
            }
        }
        return false;
    }
    static glm::vec2 get_euler_angles(glm::vec3 direction) {
        glm::vec2 angle;
        angle.x = -asin(direction.y);
        float factor = cos(angle.x);
        angle.y = atan2(direction.x / factor, direction.z / factor);
        return angle;
    }
    static std::vector<vertex> convert_face(const face& f, glm::vec3 new_normal) {
        glm::vec2 offset = get_euler_angles(new_normal) - get_euler_angles(f.normal);
        glm::mat4 rotation_matrix = glm::toMat4(glm::quat(glm::vec3(offset, 0.f)));
        std::vector<vertex> vertices;
        for (vertex v : f.vertices) {
            v.position = rotation_matrix * glm::vec4(v.position, 1.f);
            v.normal = new_normal;
            vertices.push_back(v);
        }
        return vertices;
    }
    static std::unordered_map<glm::ivec3, std::vector<vertex>, hash_vector<3, int32_t>> get_faces() {
        std::vector<glm::ivec3> offsets = {
            glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0),
            glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0),
            glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)
        };
        face f = {
            {
                { glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f) },
                { glm::vec3( 0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(1.f, 0.f) },
                { glm::vec3( 0.5f,  0.5f, 0.5f), glm::vec3(0.f), glm::vec2(1.f, 1.f) },
                { glm::vec3(-0.5f,  0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 1.f) }
            },
            glm::vec3(0.f, 0.f, 1.f)
        };
        std::unordered_map<glm::ivec3, std::vector<vertex>, hash_vector<3, int32_t>> faces;
        for (const auto& offset : offsets) {
            glm::vec3 normal = glm::vec3(offset);
            faces[offset] = convert_face(f, normal);
        }
        return faces;
    }
    mesh_factory::mesh_factory(ref<world> _world) {
        this->m_world = _world;
    }
    std::vector<std::vector<mesh_factory::processed_voxel>> mesh_factory::get_clusters(std::vector<std::pair<glm::vec3, ref<lighting::light>>>& lights) {
        lights.clear();
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
                    ref<block> block_ = block_register[block_id];
                    ref<lighting::light> light = block_->get_light();
                    if (light) {
                        lights.push_back({ glm::vec3(position), light });
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
    ref<mesh> mesh_factory::create_mesh(std::vector<processed_voxel> voxels) {
        auto faces = get_faces();
        std::vector<vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> face_indices = {
            0, 1, 3,
            1, 2, 3
        };
        for (const auto& voxel : voxels) {
            for (const std::pair<glm::ivec3, std::vector<vertex>>& pair : faces) {
                if (contains(voxel.surroundings, pair.first)) {
                    continue;
                }
                std::vector<vertex> current_vertices(pair.second.size());
                std::copy(pair.second.begin(), pair.second.end(), current_vertices.begin());
                for (vertex& v : current_vertices) {
                    v.position += glm::vec3(voxel.position);
                    size_t block_id;
                    this->m_world->get_block(voxel.position, block_id);
                    v.block_id = (int32_t)block_id;
                }
                size_t index_offset = vertices.size();
                vertices.insert(vertices.end(), current_vertices.begin(), current_vertices.end());
                std::vector<uint32_t> current_indices(face_indices.size());
                std::copy(face_indices.begin(), face_indices.end(), current_indices.begin());
                for (uint32_t& index : current_indices) {
                    index += (uint32_t)index_offset;
                }
                indices.insert(indices.end(), current_indices.begin(), current_indices.end());
            }
        }
        return ref<mesh_factory_mesh>::create(vertices, indices);
    }
    std::vector<graphics::vertex_attribute> mesh_factory::get_vertex_attributes() {
        return {
            { sizeof(vertex), offsetof(vertex, position), graphics::vertex_attribute_type::VEC3, false },
            { sizeof(vertex), offsetof(vertex, normal), graphics::vertex_attribute_type::VEC3, false },
            { sizeof(vertex), offsetof(vertex, uv), graphics::vertex_attribute_type::VEC2, false },
            { sizeof(vertex), offsetof(vertex, block_id), graphics::vertex_attribute_type::INT, false }
        };
    }
}
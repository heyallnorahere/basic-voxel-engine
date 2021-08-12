#pragma once
#include "world.h"
#include "texture_atlas.h"
namespace bve {
    enum class vertex_attribute_type {
        FLOAT,
        INT,
        VEC2,
        IVEC2,
        VEC3,
        IVEC3,
        VEC4,
        IVEC4,
        MAT4
    };
    struct vertex_attribute {
        size_t stride, offset;
        vertex_attribute_type type;
        bool normalize;
    };
    struct command_list;
    class shader;
    class mesh {
    public:
        virtual ~mesh() { }
        virtual const void* vertex_buffer_data() const = 0;
        virtual size_t vertex_buffer_data_size() const = 0;
        virtual size_t vertex_count() const = 0;
        virtual std::vector<uint32_t> index_buffer_data() const = 0;
    };
    class renderer {
    public:
        struct stats {
            uint32_t rendercalls;
        };
        renderer() = default;
        renderer(const renderer&) = delete;
        renderer& operator=(const renderer&) = delete;
        command_list* create_command_list();
        void destroy_command_list(command_list* cmdlist);
        void add_mesh(command_list* cmdlist, std::shared_ptr<mesh> mesh_);
        void close_command_list(command_list* cmdlist, const std::vector<vertex_attribute>& attributes);
        void render(command_list* cmdlist, std::shared_ptr<shader> shader_, std::shared_ptr<texture_atlas> atlas = nullptr);
        void set_camera_data(glm::vec3 position, glm::vec3 direction, float aspect_ratio, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f), float near_plane = 0.1f, float far_plane = 100.f);
        void set_camera_data(entity camera_entity, float aspect_ratio);
    private:
        glm::mat4 m_projection = glm::mat4(1.f);
        glm::mat4 m_view = glm::mat4(1.f);
    };
}
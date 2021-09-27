#pragma once
#include "world.h"
#include "texture_atlas.h"
#include "graphics/object_factory.h"
#include "lighting/light.h"
#include "buffer.h"
namespace bve {
    struct command_list;
    class mesh : public ref_counted {
    public:
        virtual ~mesh() { }
        virtual const void* vertex_buffer_data() const = 0;
        virtual size_t vertex_buffer_data_size() const = 0;
        virtual size_t vertex_count() const = 0;
        virtual std::vector<uint32_t> index_buffer_data() const = 0;
    };
    class renderer : public ref_counted {
    public:
        // see /src/assets/shaders/static.glsl
        struct vertex_uniform_buffer_t {
            glm::mat4 projection, view;
        };
        struct fragment_uniform_buffer_t {
            lighting::light::uniform_data lights[30];
            int32_t light_count;
            texture_atlas::uniform_data texture_atlas_;
            glm::vec3 camera_position;
        };
        static const size_t max_texture_units = 30;
        renderer(ref<graphics::object_factory> factory);
        renderer(const renderer&) = delete;
        renderer& operator=(const renderer&) = delete;
        command_list* create_command_list();
        void destroy_command_list(command_list* cmdlist);
        void add_mesh(command_list* cmdlist, ref<mesh> mesh_);
        void add_lights(command_list* cmdlist, const std::vector<std::pair<glm::vec3, ref<lighting::light>>>& lights);
        void close_command_list(command_list* cmdlist, const std::vector<graphics::vertex_attribute>& attributes);
        void render(command_list* cmdlist, ref<graphics::context> context, ref<texture_atlas> atlas = nullptr);
        void set_camera_data(glm::vec3 position, glm::vec3 direction, float aspect_ratio, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f), float near_plane = 0.1f, float far_plane = 100.f);
        void set_camera_data(entity camera_entity, float aspect_ratio);
        void set_shader(ref<graphics::shader> shader_);
    private:
        void set_uniform_data();
        ref<graphics::object_factory> m_factory;
        ref<graphics::uniform_buffer> m_vertex_uniform_buffer, m_fragment_uniform_buffer, m_texture_buffer;
        buffer m_vertex_uniform_data, m_fragment_uniform_data, m_sampler_data;
        std::array<ref<graphics::texture>, max_texture_units> m_textures;
        ref<graphics::texture> m_placeholder_texture;
        ref<graphics::shader> m_current_shader;
        vertex_uniform_buffer_t m_vub_data;
        fragment_uniform_buffer_t m_fub_data;
    };
}
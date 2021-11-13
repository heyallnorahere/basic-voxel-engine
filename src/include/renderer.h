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
        static const size_t max_texture_units = 30;
        renderer(ref<graphics::object_factory> factory);
        renderer(const renderer&) = delete;
        renderer& operator=(const renderer&) = delete;
        command_list* create_command_list();
        void destroy_command_list(command_list* cmdlist);
        void add_mesh(command_list* cmdlist, ref<mesh> mesh_);
        void close_command_list(command_list* cmdlist, const std::vector<graphics::vertex_attribute>& attributes);
        ref<graphics::pipeline> get_command_list_pipeline(command_list* cmdlist);
        void new_frame();
        void render(command_list* cmdlist, ref<graphics::context> context);
        void set_shader(ref<graphics::shader> shader_);
        void set_texture(size_t index, ref<graphics::texture> texture) { this->m_textures[index] = texture; }
        std::map<uint32_t, buffer>& get_uniform_buffers() { return this->m_uniform_buffers; }
    private:
        ref<graphics::object_factory> m_factory;
        std::array<ref<graphics::texture>, max_texture_units> m_textures;
        std::map<uint32_t, buffer> m_uniform_buffers;
        ref<graphics::texture> m_placeholder_texture;
        ref<graphics::shader> m_current_shader;
        std::vector<ref<graphics::uniform_buffer>> m_created_uniform_buffers;
    };
}
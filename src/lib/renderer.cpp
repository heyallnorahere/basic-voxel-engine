#include "bve_pch.h"
#include "renderer.h"
#include "components.h"
#include "lighting/light.h"
#include "asset_manager.h"
namespace bve {
    struct command_list {
        ref<graphics::buffer> vao, vbo, ebo;
        std::vector<ref<mesh>> meshes;
        std::vector<std::pair<glm::vec3, ref<lighting::light>>> lights;
        size_t index_count;
        bool open;
    };
    renderer::renderer(ref<graphics::object_factory> factory) {
        this->m_factory = factory;
        size_t sampler_buffer_size = sizeof(int32_t) * max_texture_units;
        this->m_texture_buffer = this->m_factory->create_uniform_buffer(sampler_buffer_size, 2);
        this->m_sampler_data.alloc(sampler_buffer_size);
        this->m_sampler_data.zero();
        auto& asset_manager_ = asset_manager::get();
        fs::path placeholder_path = asset_manager_.get_asset_path("block:bve:placeholder.png");
        this->m_placeholder_texture = this->m_factory->create_texture(placeholder_path);
    }
    command_list* renderer::create_command_list() {
        auto cmdlist = new command_list;
        cmdlist->open = true;
        cmdlist->index_count = 0;
        return cmdlist;
    }
    void renderer::destroy_command_list(command_list* cmdlist) {
        delete cmdlist;
    }
    void renderer::add_mesh(command_list* cmdlist, ref<mesh> mesh_) {
        if (!cmdlist->open) {
            throw std::runtime_error("[renderer] attempted to add a mesh to a closed command list");
        }
        cmdlist->meshes.push_back({ mesh_ });
    }
    void renderer::add_lights(command_list* cmdlist, const std::vector<std::pair<glm::vec3, ref<lighting::light>>>& lights) {
        cmdlist->lights.insert(cmdlist->lights.end(), lights.begin(), lights.end());
    }
    void renderer::close_command_list(command_list* cmdlist, const std::vector<graphics::vertex_attribute>& attributes) {
        if (!cmdlist->open) {
            return;
        }
        cmdlist->open = false;
        ref<graphics::vao> vao = this->m_factory->create_vao();
        vao->bind();
        cmdlist->vao = vao;
        std::vector<uint32_t> indices;
        size_t vertex_buffer_size = 0;
        size_t vertex_count = 0;
        for (const auto& mesh : cmdlist->meshes) {
            std::vector<uint32_t> mesh_indices = mesh->index_buffer_data();
            for (uint32_t& index : mesh_indices) {
                index += (uint32_t)vertex_count;
            }
            indices.insert(indices.end(), mesh_indices.begin(), mesh_indices.end());
            vertex_buffer_size += mesh->vertex_buffer_data_size();
            vertex_count += mesh->vertex_count();
        }
        cmdlist->index_count = indices.size();
        void* vertex_buffer_data = malloc(vertex_buffer_size);
        if (!vertex_buffer_data) {
            throw std::runtime_error("[renderer] ran out of memory on the heap");
        }
        size_t current_offset = 0;
        for (const auto& mesh : cmdlist->meshes) {
            size_t size = mesh->vertex_buffer_data_size();
            const void* data = mesh->vertex_buffer_data();
            void* ptr = (void*)((size_t)vertex_buffer_data + current_offset);
            memcpy(ptr, data, size);
            current_offset += size;
        }
        cmdlist->vbo = this->m_factory->create_vbo(vertex_buffer_data, vertex_buffer_size);
        cmdlist->ebo = this->m_factory->create_ebo(indices);
        vao->set_vertex_attributes(attributes);
    }
    void renderer::render(command_list* cmdlist, ref<graphics::context> context, ref<texture_atlas> atlas) {
        this->m_current_shader->bind();
        if (atlas) {
            this->m_fub_data.texture_atlas_ = atlas->get_uniform_data();
            this->m_textures[this->m_fub_data.texture_atlas_.image] = atlas->get_texture();
        }
        if (cmdlist->lights.size() > 30) {
            throw std::runtime_error("[renderer] scene cannot contain more than 30 lights!");
        }
        this->m_fub_data.light_count = (int32_t)cmdlist->lights.size();
        for (size_t i = 0; i < cmdlist->lights.size(); i++) {
            ref<lighting::light> light = cmdlist->lights[i].second;
            lighting::light::uniform_data data = light->get_uniform_data();
            data.position = cmdlist->lights[i].first;
            this->m_fub_data.lights[i] = data;
        }
        int32_t* sampler_data = this->m_sampler_data;
        for (size_t i = 0; i < max_texture_units; i++) {
            ref<graphics::texture> texture = this->m_textures[i];
            uint32_t texture_slot = (uint32_t)i;
            if (texture) {
                texture->bind(texture_slot);
            } else {
                this->m_placeholder_texture->bind(texture_slot);
            }
            sampler_data[i] = (int32_t)i;
        }
        this->set_uniform_data();
        this->m_texture_buffer->set_data(this->m_sampler_data);
        cmdlist->vao->bind();
        context->draw_indexed(cmdlist->index_count);
        cmdlist->vao->unbind();
        this->m_current_shader->unbind();
    }
    void renderer::set_camera_data(glm::vec3 position, glm::vec3 direction, float aspect_ratio, glm::vec3 up, float near_plane, float far_plane) {
        this->m_vub_data.projection = glm::perspective(glm::radians(45.f), aspect_ratio, near_plane, far_plane);
        this->m_vub_data.view = glm::lookAt(position, position + direction, up);
        this->m_fub_data.camera_position = position;
    }
    void renderer::set_camera_data(entity camera_entity, float aspect_ratio) {
        if (!camera_entity.has_component<components::camera_component>()) {
            throw std::runtime_error("[renderer] the given entity does not have a camera component");
        }
        const auto& transform = camera_entity.get_component<components::transform_component>();
        const auto& camera = camera_entity.get_component<components::camera_component>();
        this->set_camera_data(transform.translation, camera.direction, aspect_ratio, camera.up, camera.near_plane, camera.far_plane);
    }
    void renderer::set_shader(ref<graphics::shader> shader_) {
        if (shader_ && shader_ != this->m_current_shader) {
            auto reflection_data = shader_->get_reflection_data();
            size_t size = reflection_data.uniform_buffers[0].type->size;
            this->m_vertex_uniform_buffer = this->m_factory->create_uniform_buffer(size, 0);
            this->m_vertex_uniform_data.alloc(size);
            this->m_vertex_uniform_data.zero();
            size = reflection_data.uniform_buffers[1].type->size;
            this->m_fragment_uniform_buffer = this->m_factory->create_uniform_buffer(size, 1);
            this->m_fragment_uniform_data.alloc(size);
            this->m_fragment_uniform_data.zero();
        }
        this->m_current_shader = shader_;
    }
    using set_field_callback = std::function<void(const std::string&, const void*, size_t)>;
    static void set_atlas_data(const texture_atlas::uniform_data& data, set_field_callback set_field) {
        std::string base_name = "texture_atlas";
        set_field(base_name + ".image", &data.image, sizeof(int32_t));
        set_field(base_name + ".texture_size", &data.texture_size, sizeof(glm::ivec2));
        set_field(base_name + ".grid_size", &data.grid_size, sizeof(glm::ivec2));
        for (size_t i = 0; i < 64; i++) {
            std::string entry_name = base_name + ".texture_dimensions[" + std::to_string(i) + "]";
            set_field(entry_name + ".grid_position", &data.texture_dimensions[i].grid_position, sizeof(glm::ivec2));
            set_field(entry_name + ".texture_dimensions", &data.texture_dimensions[i].texture_dimensions, sizeof(glm::ivec2));
        }
    }
    void renderer::set_uniform_data() {
        auto reflection_data = this->m_current_shader->get_reflection_data();
        auto set_field = [&reflection_data](const std::string& name, const void* data, size_t size, uint32_t uniform_buffer, buffer& memory) {
            auto type = reflection_data.uniform_buffers[uniform_buffer].type;
            size_t offset = type->find_offset(name);
            spdlog::info("[renderer] uniform buffer {0}: offset of {1} is {2}", uniform_buffer, name, offset);
            memory.copy(data, size, offset);
        };
        set_field("projection", &this->m_vub_data.projection, sizeof(glm::mat4), 0, this->m_vertex_uniform_data);
        set_field("view", &this->m_vub_data.view, sizeof(glm::mat4), 0, this->m_vertex_uniform_data);
        set_field("light_count", &this->m_fub_data.light_count, sizeof(int32_t), 1, this->m_fragment_uniform_data);
        for (int32_t i = 0; i < this->m_fub_data.light_count; i++) {
            std::string light_name = "lights[" + std::to_string(i) + "]";
            auto light_data = this->m_fub_data.lights[i];
            set_field(light_name + ".type", &light_data.type, sizeof(int32_t), 1, this->m_fragment_uniform_data);
            set_field(light_name + ".position", &light_data.position, sizeof(glm::vec3), 1, this->m_fragment_uniform_data);
            set_field(light_name + ".color", &light_data.color, sizeof(glm::vec3), 1, this->m_fragment_uniform_data);
            set_field(light_name + ".ambient_strength", &light_data.ambient_strength, sizeof(float), 1, this->m_fragment_uniform_data);
            set_field(light_name + ".specular_strength", &light_data.specular_strength, sizeof(float), 1, this->m_fragment_uniform_data);
            set_field(light_name + ".direction", &light_data.direction, sizeof(glm::vec3), 1, this->m_fragment_uniform_data);
            set_field(light_name + ".cutoff", &light_data.cutoff, sizeof(float), 1, this->m_fragment_uniform_data);
        }
        set_atlas_data(this->m_fub_data.texture_atlas_, [set_field, this](const std::string& name, const void* data, size_t size) {
            set_field(name, data, size, 1, this->m_fragment_uniform_data);
        });
        set_field("camera_position", &this->m_fub_data.camera_position, sizeof(glm::vec3), 1, this->m_fragment_uniform_data);
        this->m_vertex_uniform_buffer->set_data(this->m_vertex_uniform_data);
        this->m_fragment_uniform_buffer->set_data(this->m_fragment_uniform_data);
    }
}
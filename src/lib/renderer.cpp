#include "bve_pch.h"
#include "renderer.h"
#include "components.h"
#include "lighting/light.h"
#include "asset_manager.h"
namespace bve {
    struct command_list {
        ref<graphics::pipeline> pipeline;
        ref<graphics::buffer> vertex_buffer, index_buffer;
        std::vector<ref<mesh>> meshes;
        size_t index_count;
        bool open;
    };
    renderer::renderer(ref<graphics::object_factory> factory) {
        this->m_factory = factory;
        size_t sampler_buffer_size = sizeof(int32_t) * max_texture_units;
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
    void renderer::close_command_list(command_list* cmdlist, const std::vector<graphics::vertex_attribute>& attributes) {
        if (!cmdlist->open) {
            return;
        }
        cmdlist->open = false;
        ref<graphics::pipeline> pipeline = this->m_factory->create_pipeline();
        pipeline->bind();
        cmdlist->pipeline = pipeline;
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
        cmdlist->vertex_buffer = this->m_factory->create_vertex_buffer(vertex_buffer_data, vertex_buffer_size);
        free(vertex_buffer_data);
        cmdlist->vertex_buffer->bind();
        cmdlist->index_buffer = this->m_factory->create_index_buffer(indices);
        cmdlist->index_buffer->bind();
        pipeline->set_vertex_attributes(attributes);
    }
    void renderer::new_frame() {
        this->m_created_uniform_buffers.clear();
    }
    void renderer::render(command_list* cmdlist, ref<graphics::context> context) {
        cmdlist->pipeline->bind();
        this->m_current_shader->bind();
        for (size_t i = 0; i < max_texture_units; i++) {
            ref<graphics::texture> texture = this->m_textures[i];
            uint32_t texture_slot = (uint32_t)i;
            if (texture) {
                texture->bind(texture_slot);
            } else {
                this->m_placeholder_texture->bind(texture_slot);
            }
        }
        for (const auto& [binding, data] : this->m_uniform_buffers) {
            auto uniform_buffer = this->m_factory->create_uniform_buffer(data.size(), binding);
            uniform_buffer->set_data(data);
            this->m_created_uniform_buffers.push_back(uniform_buffer); // so the ubo doesnt get cleaned up
        }
        context->draw_indexed(cmdlist->index_count);
        cmdlist->pipeline->unbind();
    }
    void renderer::set_shader(ref<graphics::shader> shader_) {
        if (shader_ && shader_ != this->m_current_shader) {
            this->m_uniform_buffers.clear();
            auto reflection_data = shader_->get_reflection_data();
            for (const auto& [binding, data] : reflection_data.uniform_buffers) {
                buffer& buffer_data = this->m_uniform_buffers[binding];
                buffer_data.alloc(data.type->size);
                buffer_data.set_dynamic_resizing_enabled(false);
            }
        }
        this->m_current_shader = shader_;
    }
}
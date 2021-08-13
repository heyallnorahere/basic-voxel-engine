#include "bve_pch.h"
#include "shader.h"
#include "renderer.h"
#include "components.h"
namespace bve {
    struct command_list {
        ref<graphics::buffer> vao, vbo, ebo;
        std::vector<ref<mesh>> meshes;
        size_t index_count;
        bool open;
    };
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
    void renderer::close_command_list(command_list* cmdlist, const std::vector<graphics::vertex_attribute>& attributes, ref<graphics::object_factory> object_factory) {
        if (!cmdlist->open) {
            return;
        }
        cmdlist->open = false;
        ref<graphics::vao> vao = object_factory->create_vao();
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
            size_t destination = (size_t)vertex_buffer_data + current_offset;
            memcpy((void*)destination, data, size);
            current_offset += size;
        }
        cmdlist->vbo = object_factory->create_vbo(vertex_buffer_data, vertex_buffer_size);
        cmdlist->ebo = object_factory->create_ebo(indices);
        free(vertex_buffer_data);
        vao->set_vertex_attributes(attributes);
    }
    void renderer::render(command_list* cmdlist, ref<shader> shader_, ref<graphics::context> context, ref<texture_atlas> atlas) {
        shader_->bind();
        shader_->set_uniform("projection", this->m_projection);
        shader_->set_uniform("view", this->m_view);
        if (atlas) {
            atlas->set_uniform(shader_, "atlas");
        }
        cmdlist->vao->bind();
        context->draw_indexed(cmdlist->index_count);
        cmdlist->vao->unbind();
        shader_->unbind();
    }
    void renderer::set_camera_data(glm::vec3 position, glm::vec3 direction, float aspect_ratio, glm::vec3 up, float near_plane, float far_plane) {
        this->m_projection = glm::perspective(glm::radians(45.f), aspect_ratio, near_plane, far_plane); // todo: add more arguments to function argument list
        this->m_view = glm::lookAt(position, position + direction, up);
    }
    void renderer::set_camera_data(entity camera_entity, float aspect_ratio) {
        if (!camera_entity.has_component<components::camera_component>()) {
            throw std::runtime_error("[renderer] the given entity does not have a camera component");
        }
        const auto& transform = camera_entity.get_component<components::transform_component>();
        const auto& camera = camera_entity.get_component<components::camera_component>();
        this->set_camera_data(transform.translation, camera.direction, aspect_ratio, camera.up, camera.near_plane, camera.far_plane);
    }
}
#include "bve_pch.h"
#include "shader.h"
#include "renderer.h"
#include "components.h"
namespace bve {
    struct command_list {
        GLuint vao, vbo, ebo;
        std::vector<std::shared_ptr<mesh>> meshes;
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
        glDeleteBuffers(1, &cmdlist->vbo);
        glDeleteBuffers(1, &cmdlist->ebo);
        glDeleteVertexArrays(1, &cmdlist->vao);
        delete cmdlist;
    }
    void renderer::add_mesh(command_list* cmdlist, std::shared_ptr<mesh> mesh_) {
        if (!cmdlist->open) {
            throw std::runtime_error("[renderer] attempted to add a mesh to a closed command list");
        }
        cmdlist->meshes.push_back({ mesh_ });
    }
    void renderer::close_command_list(command_list* cmdlist, const std::vector<vertex_attribute>& attributes) {
        if (!cmdlist->open) {
            return;
        }
        cmdlist->open = false;
        glGenVertexArrays(1, &cmdlist->vao);
        glBindVertexArray(cmdlist->vao);
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
        glGenBuffers(1, &cmdlist->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, cmdlist->vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertex_buffer_size, vertex_buffer_data, GL_STATIC_DRAW);
        glGenBuffers(1, &cmdlist->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmdlist->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(uint32_t)), indices.data(), GL_STATIC_DRAW);
        free(vertex_buffer_data);
        for (size_t i = 0; i < attributes.size(); i++) {
            const auto& attrib = attributes[i];
            bool integer;
            GLint element_count;
            switch (attrib.type) {
            case vertex_attribute_type::FLOAT:
                integer = false;
                element_count = 1;
                break;
            case vertex_attribute_type::INT:
                integer = true;
                element_count = 1;
                break;
            case vertex_attribute_type::VEC2:
                integer = false;
                element_count = 2;
                break;
            case vertex_attribute_type::IVEC2:
                integer = true;
                element_count = 2;
                break;
            case vertex_attribute_type::VEC3:
                integer = false;
                element_count = 3;
                break;
            case vertex_attribute_type::IVEC3:
                integer = true;
                element_count = 3;
                break;
            case vertex_attribute_type::VEC4:
                integer = false;
                element_count = 4;
                break;
            case vertex_attribute_type::IVEC4:
                integer = true;
                element_count = 4;
                break;
            case vertex_attribute_type::MAT4:
                integer = false;
                element_count = 16;
                break;
            default:
                throw std::runtime_error("[renderer] invalid vertex_attribute_type value");
                break;
            }
            if (integer) {
                glVertexAttribIPointer((GLuint)i, element_count, GL_INT, (GLsizei)attrib.stride, (void*)attrib.offset);
            } else {
                glVertexAttribPointer((GLuint)i, element_count, GL_FLOAT, attrib.normalize, (GLsizei)attrib.stride, (void*)attrib.offset);
            }
            glEnableVertexAttribArray((GLuint)i);
        }
        glBindVertexArray(0);
    }
    void renderer::render(command_list* cmdlist, std::shared_ptr<shader> shader_, std::shared_ptr<texture_atlas> atlas) {
        shader_->bind();
        shader_->set_uniform("projection", this->m_projection);
        shader_->set_uniform("view", this->m_view);
        if (atlas) {
            atlas->set_uniform(shader_, "atlas");
        }
        glBindVertexArray(cmdlist->vao);
        glDrawElements(GL_TRIANGLES, (GLsizei)cmdlist->index_count, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
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
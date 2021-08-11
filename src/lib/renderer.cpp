#include "bve_pch.h"
#include "shader.h"
#include "renderer.h"
#include "components.h"
namespace bve {
    struct command_list {
        GLuint vertex_array_object;
        std::vector<std::tuple<GLuint, GLuint>> meshes;
        size_t index_count;
        bool open;
    };
    command_list* renderer::create_command_list() {
        auto cmdlist = new command_list;
        cmdlist->open = true;
        cmdlist->index_count = 0;
        glGenVertexArrays(1, &cmdlist->vertex_array_object);
        return cmdlist;
    }
    void renderer::destroy_command_list(command_list* cmdlist) {
        for (auto& mesh : cmdlist->meshes) {
            GLuint buffer = std::get<0>(mesh);
            glDeleteBuffers(1, &buffer);
            buffer = std::get<1>(mesh);
            glDeleteBuffers(1, &buffer);
        }
        glDeleteVertexArrays(1, &cmdlist->vertex_array_object);
        delete cmdlist;
    }
    void renderer::add_mesh(command_list* cmdlist, GLuint vertex_buffer, GLuint index_buffer, size_t index_count) {
        if (!cmdlist->open) {
            throw std::runtime_error("[renderer] attempted to add a mesh to a closed command list");
        }
        glBindVertexArray(cmdlist->vertex_array_object);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBindVertexArray(0);
        cmdlist->index_count += index_count;
        cmdlist->meshes.push_back({ vertex_buffer, index_buffer });
    }
    void renderer::close_command_list(command_list* cmdlist, const std::vector<vertex_attribute>& attributes) {
        if (!cmdlist->open) {
            return;
        }
        cmdlist->open = false;
        glBindVertexArray(cmdlist->vertex_array_object);
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
    void renderer::render(command_list* cmdlist, std::shared_ptr<shader> shader_) {
        shader_->bind();
        shader_->set_uniform("projection", this->m_projection);
        shader_->set_uniform("view", this->m_view);
        glBindVertexArray(cmdlist->vertex_array_object);
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
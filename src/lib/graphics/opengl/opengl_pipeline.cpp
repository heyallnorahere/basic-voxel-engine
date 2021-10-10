#include "bve_pch.h"
#include "opengl_pipeline.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            opengl_pipeline::opengl_pipeline() {
                glGenVertexArrays(1, &this->m_id);
            }
            opengl_pipeline::~opengl_pipeline() {
                glDeleteVertexArrays(1, &this->m_id);
            }
            void opengl_pipeline::bind() {
                glBindVertexArray(this->m_id);
            }
            void opengl_pipeline::unbind() {
                glBindVertexArray(0);
            }
            void opengl_pipeline::set_vertex_attributes(const std::vector<vertex_attribute>& attributes) {
                this->bind();
                for (size_t i = 0; i < attributes.size(); i++) {
                    const auto& attrib = attributes[i];
                    bool integer;
                    int32_t element_count;
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
                        throw std::runtime_error("[opengl pipeline] invalid vertex_attribute_type value");
                        break;
                    }
                    if (integer) {
                        glVertexAttribIPointer((GLuint)i, element_count, GL_INT, (GLsizei)attrib.stride, (void*)attrib.offset);
                    }
                    else {
                        glVertexAttribPointer((GLuint)i, element_count, GL_FLOAT, attrib.normalize, (GLsizei)attrib.stride, (void*)attrib.offset);
                    }
                    glEnableVertexAttribArray((GLuint)i);
                }
                this->unbind();
            }
        }
    }
}
#include "bve_pch.h"
#include "opengl_buffer.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            opengl_buffer::opengl_buffer(const void* data, size_t size, GLenum target) {
                this->m_target = target;
                glGenBuffers(1, &this->m_id);
                glBindBuffer(this->m_target, this->m_id);
                glBufferData(this->m_target, (GLsizeiptr)size, data, GL_STATIC_DRAW);
            }
            opengl_buffer::~opengl_buffer() {
                glDeleteBuffers(1, &this->m_id);
            }
            void opengl_buffer::bind() {
                glBindBuffer(this->m_target, this->m_id);
            }
            void opengl_buffer::unbind() {
                glBindBuffer(this->m_target, 0);
            }
        }
    }
}
#include "bve_pch.h"
#include "opengl_uniform_buffer.h"
#include "opengl_context.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            opengl_uniform_buffer::opengl_uniform_buffer(size_t size, uint32_t binding) {
                this->m_size = size;
                this->m_binding = binding;
                if (opengl_context::get_version() < 4.5) {
                    throw std::runtime_error("[opengl uniform buffer] glNamedBufferData and glCreateBuffers are only supported in version 4.5 and later");
                }
                glCreateBuffers(1, &this->m_id);
                glNamedBufferData(this->m_id, (GLsizeiptr)this->m_size, nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, this->m_binding, this->m_id);
            }
            opengl_uniform_buffer::~opengl_uniform_buffer() {
                glDeleteBuffers(1, &this->m_id);
            }
            void opengl_uniform_buffer::set_data(const void* data, size_t size, size_t offset) {
                glNamedBufferSubData(this->m_id, (GLintptr)offset, (GLsizeiptr)size, data);
            }
        }
    }
}
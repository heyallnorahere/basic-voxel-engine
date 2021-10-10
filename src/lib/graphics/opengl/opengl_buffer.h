#pragma once
#include "graphics/buffer.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            class opengl_buffer : public buffer {
            public:
                opengl_buffer(const void* data, size_t size, GLenum target);
                virtual ~opengl_buffer() override;
                virtual void bind() override;
                virtual void unbind() override;
            private:
                GLenum m_target;
                uint32_t m_id;
            };
        }
    }
}
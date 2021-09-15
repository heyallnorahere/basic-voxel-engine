#pragma once
#include "graphics/uniform_buffer.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            class opengl_uniform_buffer : public uniform_buffer {
            public:
                opengl_uniform_buffer(size_t size, uint32_t binding);
                virtual ~opengl_uniform_buffer() override;
                virtual void set_data(const void* data, size_t size, size_t offset) override;
            private:
                uint32_t m_id;
            };
        }
    }
}
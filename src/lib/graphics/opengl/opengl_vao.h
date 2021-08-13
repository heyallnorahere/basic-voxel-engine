#pragma once
#include "graphics/vao.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            class opengl_vao : public vao {
            public:
                opengl_vao();
                virtual ~opengl_vao() override;
                virtual void bind() override;
                virtual void unbind() override;
                virtual void set_vertex_attributes(const std::vector<vertex_attribute>& attributes) override;
            private:
                GLuint m_id;
            };
        }
    }
}
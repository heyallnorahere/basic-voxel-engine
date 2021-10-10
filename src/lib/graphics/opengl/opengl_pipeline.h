#pragma once
#include "graphics/pipeline.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            class opengl_pipeline : public pipeline {
            public:
                opengl_pipeline();
                virtual ~opengl_pipeline() override;
                virtual void bind() override;
                virtual void unbind() override;
                virtual void set_vertex_attributes(const std::vector<vertex_attribute>& attributes) override;
            private:
                uint32_t m_id;
            };
        }
    }
}
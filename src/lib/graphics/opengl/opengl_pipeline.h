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
                virtual void push_constants(shader_type stage, const void* data, size_t size, size_t offset) override { } // is that even a feature in opengl?
            private:
                uint32_t m_id;
            };
        }
    }
}
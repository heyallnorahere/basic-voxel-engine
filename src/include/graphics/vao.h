#pragma once
#include "buffer.h"
namespace bve {
    namespace graphics {
        enum class vertex_attribute_type {
            FLOAT,
            INT,
            VEC2,
            IVEC2,
            VEC3,
            IVEC3,
            VEC4,
            IVEC4,
            MAT4
        };
        struct vertex_attribute {
            size_t stride, offset;
            vertex_attribute_type type;
            bool normalize;
        };
        class vao : public buffer {
        public:
            vao() = default;
            vao(const vao&) = delete;
            vao& operator=(const vao&) = delete;
            virtual void set_vertex_attributes(const std::vector<vertex_attribute>& attributes) = 0;
        };
    }
}
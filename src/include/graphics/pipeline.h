#pragma once
namespace bve {
    namespace graphics {
        enum class vertex_attribute_type {
            FLOAT = 1 << 0,
            INT = 1 << 1,
            VEC2 = 1 << 2,
            IVEC2 = 1 << 3,
            VEC3 = 1 << 4,
            IVEC3 = 1 << 5,
            VEC4 = 1 << 6,
            IVEC4 = 1 << 7,
            MAT4 = 1 << 8
        };
        struct vertex_attribute {
            size_t stride, offset;
            vertex_attribute_type type;
            bool normalize;
        };
        class pipeline : public ref_counted {
        public:
            pipeline() = default;
            virtual ~pipeline() = default;
            pipeline(const pipeline&) = delete;
            pipeline& operator=(const pipeline&) = delete;
            virtual void set_vertex_attributes(const std::vector<vertex_attribute>& attributes) = 0;
            virtual void bind() = 0;
            virtual void unbind() = 0;
        };
    }
}
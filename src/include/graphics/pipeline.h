#pragma once
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
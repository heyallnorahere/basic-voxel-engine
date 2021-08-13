#pragma once
#include "graphics/object_factory.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            class opengl_object_factory : public object_factory {
            public:
                virtual ref<vao> create_vao() override;
                virtual ref<buffer> create_vbo(const void* data, size_t size) override;
                virtual ref<buffer> create_ebo(const std::vector<uint32_t>& data) override;
                virtual ref<context> create_context() override;
            };
        }
    }
}
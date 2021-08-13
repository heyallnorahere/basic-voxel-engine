#pragma once
#include "buffer.h"
#include "context.h"
#include "vao.h"
namespace bve {
    namespace graphics {
        enum class graphics_api {
            OPENGL
        };
        class object_factory : public ref_counted {
        public:
            static ref<object_factory> create(graphics_api api);
            object_factory() = default;
            virtual ~object_factory() = default;
            object_factory(const object_factory&) = delete;
            object_factory& operator=(const object_factory&) = delete;
            virtual ref<vao> create_vao() = 0;
            template<typename T> ref<buffer> create_vbo(const std::vector<T>& data) {
                return this->create_vbo(data.data(), data.size() * sizeof(T));
            }
            virtual ref<buffer> create_vbo(const void* data, size_t size) = 0;
            virtual ref<buffer> create_ebo(const std::vector<uint32_t>& data) = 0;
            virtual ref<context> create_context() = 0;
        };
    }
}
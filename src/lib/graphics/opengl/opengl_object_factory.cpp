#include "bve_pch.h"
#include "opengl_object_factory.h"
#include "opengl_vao.h"
#include "opengl_buffer.h"
#include "opengl_context.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            ref<vao> opengl_object_factory::create_vao() {
                return ref<opengl_vao>::create();
            }
            ref<buffer> opengl_object_factory::create_vbo(const void* data, size_t size) {
                return ref<opengl_buffer>::create(data, size, GL_ARRAY_BUFFER);
            }
            ref<buffer> opengl_object_factory::create_ebo(const std::vector<uint32_t>& data) {
                return ref<opengl_buffer>::create(data.data(), data.size() * sizeof(uint32_t), GL_ELEMENT_ARRAY_BUFFER);
            }
            ref<context> opengl_object_factory::create_context() {
                return ref<opengl_context>::create();
            }
        }
    }
}
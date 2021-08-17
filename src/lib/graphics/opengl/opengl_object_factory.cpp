#include "bve_pch.h"
#include "opengl_object_factory.h"
#include "opengl_vao.h"
#include "opengl_buffer.h"
#include "opengl_context.h"
#include "opengl_shader.h"
#include "opengl_texture.h"
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
            ref<shader> opengl_object_factory::create_shader(const std::vector<std::filesystem::path>& sources) {
                return ref<opengl_shader>::create(sources);
            }
            ref<texture> opengl_object_factory::create_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) {
                return ref<opengl_texture>::create(data, width, height, channels, opengl_texture_settings{ });
            }
        }
    }
}
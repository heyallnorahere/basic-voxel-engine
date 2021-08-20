#include "bve_pch.h"
#include "vulkan_object_factory.h"
#include "vulkan_context.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            ref<vao> vulkan_object_factory::create_vao() {
                // todo: create vao
                return nullptr;
            }
            ref<buffer> vulkan_object_factory::create_vbo(const void* data, size_t size) {
                // todo: create vbo
                return nullptr;
            }
            ref<buffer> vulkan_object_factory::create_ebo(const std::vector<uint32_t>& data) {
                // todo: create ebo
                return nullptr;
            }
            ref<context> vulkan_object_factory::create_context() {
                return ref<vulkan_context>::create(this);
            }
            ref<shader> vulkan_object_factory::create_shader(const std::vector<std::filesystem::path>& sources) {
                // todo: create shader
                return nullptr;
            }
            ref<texture> vulkan_object_factory::create_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) {
                // todo: create texture
                return nullptr;
            }
        }
    }
}
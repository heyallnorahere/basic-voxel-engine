#include "bve_pch.h"
#include "vulkan_object_factory.h"
#include "vulkan_context.h"
#include "vulkan_shader.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            ref<pipeline> vulkan_object_factory::create_pipeline() {
                return ref<vulkan_pipeline>::create(this);
            }
            ref<buffer> vulkan_object_factory::create_vertex_buffer(const void* data, size_t size) {
                return ref<vulkan_buffer>::create(data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, this);
            }
            ref<buffer> vulkan_object_factory::create_index_buffer(const std::vector<uint32_t>& data) {
                return ref<vulkan_buffer>::create(data.data(), data.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this);
            }
            ref<context> vulkan_object_factory::create_context() {
                return ref<vulkan_context>::create(this);
            }
            ref<shader> vulkan_object_factory::create_shader(const std::vector<fs::path>& sources) {
                return ref<vulkan_shader>::create(this, sources);
            }
            ref<texture> vulkan_object_factory::create_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) {
                // todo: create texture
                return nullptr;
            }
            ref<uniform_buffer> vulkan_object_factory::create_uniform_buffer(size_t size, uint32_t binding) {
                // todo: create uniform buffer
                return nullptr;
            }
            ref<context> vulkan_object_factory::get_current_context() {
                return this->m_current_context;
            }
            ref<pipeline> vulkan_object_factory::get_current_pipeline() {
                return this->m_current_pipeline;
            }
        }
    }
}
#include "bve_pch.h"
#include "dx12_object_factory.h"
#include "dx12_util.h"
#include "dx12_context.h"
namespace bve {
    namespace graphics {
        namespace dx12 {
            dx12_object_factory::dx12_object_factory() {
                this->m_current_context = nullptr;
                this->m_current_pipeline = nullptr;
            }
            ref<pipeline> dx12_object_factory::create_pipeline() {
                return nullptr; // todo: create pipeline
            }
            ref<buffer> dx12_object_factory::create_vertex_buffer(const void* data, size_t size) {
                return nullptr; // todo: create vertex buffer
            }
            ref<buffer> dx12_object_factory::create_index_buffer(const std::vector<uint32_t>& data) {
                return nullptr; // todo: create index buffer
            }
            ref<context> dx12_object_factory::create_context() {
                return ref<dx12_context>::create(this);
            }
            ref<shader> dx12_object_factory::create_shader(const std::vector<fs::path>& sources) {
                return nullptr; // todo: create shader
            }
            ref<texture> dx12_object_factory::create_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) {
                return nullptr; // todo: create texture
            }
            ref<uniform_buffer> dx12_object_factory::create_uniform_buffer(size_t size, uint32_t binding) {
                return nullptr; // todo: create uniform buffer? if those exist in dx?
            }
        }
    }
}
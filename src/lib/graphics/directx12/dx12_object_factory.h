#pragma once
#include "graphics/object_factory.h"
namespace bve {
    namespace graphics {
        namespace dx12 {
            class dx12_object_factory : public object_factory {
            public:
                virtual ref<pipeline> create_pipeline() override;
                virtual ref<buffer> create_vertex_buffer(const void* data, size_t size) override;
                virtual ref<buffer> create_index_buffer(const std::vector<uint32_t>& data) override;
                virtual ref<context> create_context() override;
                virtual ref<shader> create_shader(const std::vector<fs::path>& sources) override;
                virtual ref<texture> create_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) override;
                virtual ref<uniform_buffer> create_uniform_buffer(size_t size, uint32_t binding) override;
            };
        }
    }
}
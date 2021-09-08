#pragma once
#include "graphics/object_factory.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_context;
            class vulkan_object_factory : public object_factory {
            public:
                virtual ref<vao> create_vao() override;
                virtual ref<buffer> create_vbo(const void* data, size_t size) override;
                virtual ref<buffer> create_ebo(const std::vector<uint32_t>& data) override;
                virtual ref<context> create_context() override;
                virtual ref<shader> create_shader(const std::vector<fs::path>& sources) override;
                virtual ref<texture> create_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) override;
                ref<context> get_current_context();
            private:
                ref<context> m_current_context;
                friend class vulkan_context;
            };
        }
    }
}
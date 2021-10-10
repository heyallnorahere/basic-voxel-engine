#pragma once
#include "dx12_object_factory.h"
namespace bve {
    namespace graphics {
        namespace dx12 {
            constexpr uint32_t backbuffer_count = 3;
            class dx12_context : public context {
            public:
                dx12_context(ref<dx12_object_factory> factory);
                virtual ~dx12_context() override;
                virtual void clear(glm::vec4 clear_color) override;
                virtual void make_current() override;
                virtual void draw_indexed(size_t index_count) override;
                ComPtr<ID3D12Device2> get_device() { return this->m_device; }
            private:
                virtual void swap_buffers() override;
                virtual void setup_glfw() override;
                virtual void setup_context() override;
                virtual void resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) override;
                virtual void init_imgui_backends() override;
                virtual void shutdown_imgui_backends() override;
                virtual void call_imgui_backend_newframe() override;
                virtual void render_imgui_draw_data(ImDrawData* data) override;
                void enable_debug_layer();
                void create_device();
                ComPtr<ID3D12Device2> m_device;
                ref<dx12_object_factory> m_factory;
            };
        }
    }
}
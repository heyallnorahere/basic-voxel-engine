#pragma once
#include "graphics/context.h"
#include "vulkan_object_factory.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_context : public context {
            public:
                vulkan_context(ref<vulkan_object_factory> factory);
                virtual void clear() override;
                virtual void make_current() override;
                virtual void draw_indexed(size_t index_count) override;
            private:
                virtual void swap_buffers() override;
                virtual void setup_glfw() override;
                virtual void setup_context() override;
                virtual void resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) override;
                virtual void init_imgui_backends() override;
                virtual void shutdown_imgui_backends() override;
                virtual void call_imgui_backend_newframe() override;
                ref<vulkan_object_factory> m_factory;
            };
        }
    }
}
#pragma once
namespace bve {
    class window;
    namespace graphics {
        class context : public ref_counted {
        public:
            context() = default;
            virtual ~context() = default;
            context(const context&) = delete;
            context& operator=(const context&) = delete;
            virtual void clear() = 0;
            virtual void make_current() = 0;
            virtual void draw_indexed(size_t index_count) = 0;
        protected:
            virtual void swap_buffers() = 0;
            virtual void setup_glfw() = 0;
            virtual void setup_context() = 0;
            virtual void resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) = 0;
            virtual void init_imgui_backends() = 0;
            virtual void shutdown_imgui_backends() = 0;
            virtual void call_imgui_backend_newframe() = 0;
            virtual void render_imgui_draw_data(ImDrawData* data) = 0;
            GLFWwindow* m_window = nullptr;
            friend class ::bve::window;
        };
    }
}
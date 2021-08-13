#pragma once
#include "graphics/context.h"
namespace bve {
    class input_manager;
    class window : public ref_counted {
    public:
        window(int32_t width, int32_t height, ref<graphics::context> context);
        ~window();
        bool should_close() const;
        void new_frame();
        void swap_buffers();
        ref<graphics::context> get_context();
        glm::ivec2 get_framebuffer_size() const;
        static void poll_events();
    private:
        GLFWwindow* m_window;
        glm::ivec2 m_framebuffer_size;
        ref<graphics::context> m_context;
        static void framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height);
        friend class input_manager;
    };
}
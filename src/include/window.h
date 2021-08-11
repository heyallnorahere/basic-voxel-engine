#pragma once
namespace bve {
    class window {
    public:
        window(int32_t width, int32_t height);
        ~window();
        bool should_close() const;
        void new_frame();
        void clear() const;
        void swap_buffers() const;
        glm::ivec2 get_size() const;
        static void poll_events();
    private:
        GLFWwindow* m_window;
        std::vector<glm::ivec2> m_sizes;
        static void framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height);
    };
}
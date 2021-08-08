#pragma once
namespace bve {
    class window {
    public:
        window(int32_t width, int32_t height);
        ~window();
    private:
        GLFWwindow* m_window;
    };
}
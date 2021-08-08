#include "bve_pch.h"
#include "window.h"
namespace bve {
    static uint32_t window_count = 0;
    static void throw_glfw_error() {
        const char* msg;
        int32_t error = glfwGetError(&msg);
        throw std::runtime_error("[window] glfw error " + std::to_string(error) + ": " + (msg != nullptr ? msg : "unspecified"));
    }
    static void increase_window_count() {
        if (window_count == 0) {
            if (!glfwInit()) {
                throw_glfw_error();
            }
        }
        window_count++;
    }
    static void decrease_window_count() {
        window_count--;
        if (window_count == 0) {
            glfwTerminate();
        }
    }
    static void setup_context() {
        glEnable(GL_DEPTH_TEST);
        // todo: enable more features as they are needed
    }
    window::window(int32_t width, int32_t height) {
        increase_window_count();
        spdlog::info("[window] creating window...");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        this->m_window = glfwCreateWindow(width, height, "basic voxel engine", nullptr, nullptr);
        if (this->m_window == nullptr) {
            throw_glfw_error();
        }
        glfwMakeContextCurrent(this->m_window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        setup_context();
        spdlog::info("[window] successfully created window!");
    }
    window::~window() {
        spdlog::info("[window] destroying window...");
        glfwDestroyWindow(this->m_window);
        decrease_window_count();
    }
    bool window::should_close() const {
        return (bool)glfwWindowShouldClose(this->m_window);
    }
    void window::clear() const {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void window::swap_buffers() const {
        glfwSwapBuffers(this->m_window);
    }
    void window::poll_events() {
        glfwPollEvents();
    }
}
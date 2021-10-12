#pragma once
namespace bve {
    namespace util {
        inline void zero(void* data, size_t size) {
            ::memset(data, 0, size);
        }
        template<typename T> inline void zero(T& data) {
            ::bve::util::zero(&data, sizeof(T));
        }
        inline glm::ivec2 get_new_window_size(GLFWwindow* window) {
            int32_t width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            if (width == 0 || height == 0) {
                // why do we do this again?
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }
            return glm::ivec2(width, height);
        }
    }
}
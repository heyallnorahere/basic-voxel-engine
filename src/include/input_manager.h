#pragma once
#include "window.h"
namespace bve {
    class input_manager {
    public:
        struct key_state {
            bool up, down, held;
            void reset() {
                this->up = this->down = this->held = false;
            }
        };
        input_manager(std::shared_ptr<window> window_);
        ~input_manager();
        input_manager(const input_manager&) = delete;
        input_manager& operator=(const input_manager&) = delete;
        key_state get_key(int32_t glfw_key);
        glm::vec2 get_mouse_offset();
        void update();
        bool& mouse_enabled();
    private:
        static void mouse_callback(GLFWwindow* glfw_window, double x, double y);
        std::shared_ptr<window> m_window;
        std::map<int32_t, key_state> m_states;
        glm::vec2 m_temp_mouse_offset, m_mouse_offset;
        bool m_mouse_enabled;
    };
}
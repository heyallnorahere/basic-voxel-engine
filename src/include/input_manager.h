#pragma once
#include "window.h"
namespace bve {
    class input_manager : public ref_counted {
    public:
        struct key_state {
            bool up, down, held;
            void reset() {
                this->up = this->down = this->held = false;
            }
        };
        input_manager(ref<window> window_);
        ~input_manager();
        input_manager(const input_manager&) = delete;
        input_manager& operator=(const input_manager&) = delete;
        key_state get_key(int32_t glfw_key);
        glm::vec2 get_mouse();
        void update();
        bool& mouse_enabled();
    private:
        static void mouse_callback(GLFWwindow* glfw_window, double x, double y);
        ref<window> m_window;
        std::map<int32_t, key_state> m_states;
        glm::vec2 m_last_mouse, m_mouse, m_current_offset;
        bool m_mouse_enabled;
    };
}
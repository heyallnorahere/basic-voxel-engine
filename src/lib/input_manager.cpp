#include "bve_pch.h"
#include "input_manager.h"
namespace bve {
#define key(KEY) GLFW_KEY_##KEY
    static const std::vector<int32_t> keys = {
        key(ESCAPE), key(F1), key(F2), key(F3), key(F4), key(F5), key(F6), key(F7), key(F8), key(F9), key(F10), key(F11), key(F12),
        key(GRAVE_ACCENT), key(1), key(2), key(3), key(4), key(5), key(6), key(7), key(8), key(9), key(0), key(MINUS), key(EQUAL), key(BACKSPACE),
        key(TAB), key(Q), key(W), key(E), key(R), key(T), key(Y), key(U), key(I), key(O), key(P), key(LEFT_BRACKET), key(RIGHT_BRACKET), key(BACKSLASH),
        key(CAPS_LOCK), key(A), key(S), key(D), key(F), key(G), key(H), key(J), key(K), key(L), key(SEMICOLON), key(APOSTROPHE), key(ENTER),
        key(LEFT_SHIFT), key(Z), key(X), key(C), key(V), key(B), key(N), key(M), key(COMMA), key(PERIOD), key(SLASH), key(RIGHT_SHIFT),
        key(LEFT_CONTROL), key(LEFT_SUPER), key(LEFT_ALT), key(SPACE), key(RIGHT_ALT), key(RIGHT_SUPER), key(RIGHT_CONTROL)
    };
#undef key
    static std::map<GLFWwindow*, input_manager*> window_map;
    input_manager::input_manager(ref<window> window_) {
        this->m_mouse = this->m_last_mouse = this->m_current_offset = glm::vec2(0.f);
        this->m_window = window_;
        this->m_mouse_enabled = false;
        window_map.insert({ this->m_window->m_window, this });
        glfwSetCursorPosCallback(this->m_window->m_window, mouse_callback);
        for (int32_t key : keys) {
            key_state state;
            state.reset();
            this->m_states.insert({ key, state });
        }
        for (size_t i = 0; i < 8; i++) {
            key_state state;
            state.reset();
            this->m_mouse_buttons.push_back(state);
        }
    }
    input_manager::~input_manager() {
        window_map.erase(this->m_window->m_window);
    }
    input_manager::key_state input_manager::get_key(int32_t glfw_key) {
        return this->m_states[glfw_key];
    }
    input_manager::key_state input_manager::get_mouse_button(size_t button) {
        return this->m_mouse_buttons[button];
    }
    glm::vec2 input_manager::get_mouse() {
        return this->m_current_offset;
    }
    static void finalize_state(input_manager::key_state& current, const input_manager::key_state& old) {
        current.down = current.held && !old.held;
        current.up = !current.held && old.held;
    }
    void input_manager::update() {
        glfwSetInputMode(this->m_window->m_window, GLFW_CURSOR, this->m_mouse_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        this->m_current_offset = (this->m_mouse - this->m_last_mouse) * glm::vec2(1.f, -1.f);
        this->m_last_mouse = this->m_mouse;
        for (int32_t key : keys) {
            key_state old_state = this->m_states[key];
            key_state& state = this->m_states[key];
            state.reset();
            state.held = glfwGetKey(this->m_window->m_window, key) == GLFW_PRESS;
            finalize_state(state, old_state);
        }
        for (size_t i = 0; i < this->m_mouse_buttons.size(); i++) {
            key_state old_state = this->m_mouse_buttons[i];
            key_state& state = this->m_mouse_buttons[i];
            state.reset();
            state.held = glfwGetMouseButton(this->m_window->m_window, (int32_t)i) == GLFW_PRESS;
            finalize_state(state, old_state);
        }
    }
    bool& input_manager::mouse_enabled() {
        return this->m_mouse_enabled;
    }
    void input_manager::mouse_callback(GLFWwindow* glfw_window, double x, double y) {
        input_manager* im = window_map[glfw_window];
        im->m_mouse = glm::vec2((float)x, (float)y);
    }
}
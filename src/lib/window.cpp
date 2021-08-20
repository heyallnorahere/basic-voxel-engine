#include "bve_pch.h"
#include "window.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
namespace bve {
    std::unordered_map<GLFWwindow*, window*> window_map;
    static uint32_t window_count = 0;
    static bool imgui_initialized = false;
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
    static void init_imgui(std::function<void()> init_backends) {
        spdlog::info("[window] initializing imgui...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.f;
            style.Colors[ImGuiCol_WindowBg].w = 1.f;
        }
        // todo: write custom style
        init_backends();
        spdlog::info("[window] successfully initialized imgui");
        imgui_initialized = true;
    }
    static void shutdown_imgui(std::function<void()> shutdown_backends) {
        if (!imgui_initialized) {
            return;
        }
        shutdown_backends();
        ImGui::DestroyContext();
        imgui_initialized = false;
    }
    window::window(int32_t width, int32_t height, ref<graphics::context> context) {
        increase_window_count();
        this->m_context = context;
        spdlog::info("[window] creating window...");
        this->m_context->setup_glfw();
        this->m_window = glfwCreateWindow(width, height, "basic voxel engine", nullptr, nullptr);
        if (this->m_window == nullptr) {
            throw_glfw_error();
        }
        this->m_context->m_window = this->m_window;
        window_map.insert({ this->m_window, this });
        glfwGetFramebufferSize(this->m_window, &this->m_framebuffer_size.x, &this->m_framebuffer_size.y);
        glfwSetFramebufferSizeCallback(this->m_window, framebuffer_size_callback);
        this->m_context->make_current();
        this->m_context->setup_context();
        spdlog::info("[window] successfully created window!");
        init_imgui([this]() { this->m_context->init_imgui_backends(); });
    }
    window::~window() {
        shutdown_imgui([this]() { this->m_context->shutdown_imgui_backends(); });
        spdlog::info("[window] destroying window...");
        glfwDestroyWindow(this->m_window);
        window_map.erase(this->m_window);
        decrease_window_count();
    }
    bool window::should_close() const {
        return (bool)glfwWindowShouldClose(this->m_window);
    }
    void window::new_frame() {
        this->m_context->call_imgui_backend_newframe();
        ImGui::NewFrame();
    }
    void window::swap_buffers() {
        ImGuiIO& io = ImGui::GetIO();
        int32_t width, height;
        glfwGetFramebufferSize(this->m_window, &width, &height);
        io.DisplaySize.x = (float)width;
        io.DisplaySize.y = (float)height;
        ImGui::Render();
        this->m_context->render_imgui_draw_data(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            this->m_context->make_current();
        }
        this->m_context->swap_buffers();
    }
    ref<graphics::context> window::get_context() {
        return this->m_context;
    }
    glm::ivec2 window::get_framebuffer_size() const {
        return this->m_framebuffer_size;
    }
    void window::poll_events() {
        glfwPollEvents();
    }
    void window::framebuffer_size_callback(GLFWwindow* glfw_window, int32_t width, int32_t height) {
        window* window_ = window_map[glfw_window];
        constexpr bool preserve_aspect_ratio = false; // temporarily
        int32_t x, y, viewport_width, viewport_height;
        if (preserve_aspect_ratio) {
            float scale = (float)height / (float)window_->m_framebuffer_size.y;
            int32_t new_width = (int32_t)(scale * (float)window_->m_framebuffer_size.x);
            x = abs(width - new_width) / 2;
            y = 0;
            viewport_width = new_width;
            viewport_height = height;
        } else {
            x = 0;
            y = 0;
            viewport_width = width;
            viewport_height = height;
        }
        window_->m_framebuffer_size = glm::ivec2(viewport_width, height);
        window_->m_context->make_current();
        window_->m_context->resize_viewport(x, y, viewport_width, viewport_height);
    }
}
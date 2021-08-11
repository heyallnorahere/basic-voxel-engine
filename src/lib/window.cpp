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
    static void setup_context() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // todo: enable more features as they are needed
    }
    static void init_imgui(GLFWwindow* glfw_window) {
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
        ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
        spdlog::info("[window] successfully initialized imgui");
        imgui_initialized = true;
    }
    static void shutdown_imgui() {
        if (!imgui_initialized) {
            return;
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        imgui_initialized = false;
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
        this->m_sizes.push_back(glm::ivec2(width, height));
        window_map.insert({ this->m_window, this });
        glfwSetFramebufferSizeCallback(this->m_window, framebuffer_size_callback);
        glfwMakeContextCurrent(this->m_window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        setup_context();
        spdlog::info("[window] successfully created window!");
        init_imgui(this->m_window);
    }
    window::~window() {
        shutdown_imgui();
        spdlog::info("[window] destroying window...");
        glfwDestroyWindow(this->m_window);
        window_map.erase(this->m_window);
        decrease_window_count();
    }
    bool window::should_close() const {
        return (bool)glfwWindowShouldClose(this->m_window);
    }
    void window::new_frame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    void window::clear() const {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void window::swap_buffers() const {
        ImGuiIO& io = ImGui::GetIO();
        int32_t width, height;
        glfwGetFramebufferSize(this->m_window, &width, &height);
        io.DisplaySize.x = (float)width;
        io.DisplaySize.y = (float)height;
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(this->m_window);
        }
        glfwSwapBuffers(this->m_window);
    }
    glm::ivec2 window::get_size() const {
        glm::ivec2 size;
        glfwGetFramebufferSize(this->m_window, &size.x, &size.y);
        return size;
    }
    void window::poll_events() {
        glfwPollEvents();
    }
    void window::framebuffer_size_callback(GLFWwindow* glfw_window, int32_t width, int32_t height) {
        window* window_ = window_map[glfw_window];
        glm::ivec2 last_size = window_->m_sizes[window_->m_sizes.size() - 1];
        glm::ivec2 new_size;
        new_size.y = height;
        float scale = (float)new_size.y / (float)last_size.y;
        new_size.x = (int32_t)(scale * (float)last_size.x);
        window_->m_sizes.push_back(new_size);
        glfwMakeContextCurrent(glfw_window);
        glViewport(static_cast<GLint>(width - new_size.x) / 2, 0, (GLsizei)new_size.x, (GLsizei)new_size.y);
    }
}
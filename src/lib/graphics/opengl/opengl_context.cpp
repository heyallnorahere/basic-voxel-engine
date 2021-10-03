#include "bve_pch.h"
#include "opengl_context.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
namespace bve {
    namespace graphics {
        namespace opengl {
            double opengl_context::get_version() {
                const char* string = (char*)glGetString(GL_VERSION);
                int32_t major, minor;
                sscanf(string, "%d.%d", &major, &minor);
                return (double)major + ((double)minor / 10);
            }
            static void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param) {
                std::string message_ = "[opengl context] OpenGL: " + std::string(message);
                switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH:
                    spdlog::error(message_);
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM:
                    spdlog::warn(message_);
                    break;
                case GL_DEBUG_SEVERITY_LOW:
                    spdlog::info(message_);
                    break;
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                    spdlog::debug(message_);
                    break;
                default:
                    return;
                }
            }
            void opengl_context::clear(glm::vec4 clear_color) {
                glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
            void opengl_context::make_current() {
                glfwMakeContextCurrent(this->m_window);
            }
            void opengl_context::draw_indexed(size_t index_count) {
                glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, nullptr);
            }
            void opengl_context::swap_buffers() {
                glfwSwapBuffers(this->m_window);
            }
            void opengl_context::setup_glfw() {
#ifdef BVE_PLATFORM_MACOSX
                throw std::runtime_error("[opengl context] OpenGL version 4.6 is not available on this platform!");
#endif
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#ifndef NDEBUG
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
            }
            void opengl_context::setup_context() {
                gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glFrontFace(GL_CCW);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                double context_version = get_version();
                int32_t context_flags;
                glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
                if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT && context_version >= 4.3) {
                    glEnable(GL_DEBUG_OUTPUT);
                    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                    glDebugMessageCallback(debug_callback, nullptr);
                    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
                }
                spdlog::info("[opengl context] created opengl context with version {}", context_version);
            }
            void opengl_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                glViewport(x, y, width, height);
            }
            void opengl_context::init_imgui_backends() {
                ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);
                ImGui_ImplOpenGL3_Init("#version 330 core");
            }
            void opengl_context::shutdown_imgui_backends() {
                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplGlfw_Shutdown();
            }
            void opengl_context::call_imgui_backend_newframe() {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
            }
            void opengl_context::render_imgui_draw_data(ImDrawData* data) {
                ImGui_ImplOpenGL3_RenderDrawData(data);
            }
        }
    }
}
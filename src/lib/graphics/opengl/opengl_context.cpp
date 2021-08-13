#include "bve_pch.h"
#include "opengl_context.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
namespace bve {
    namespace graphics {
        namespace opengl {
            void opengl_context::clear() {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
            void opengl_context::make_current() {
                glfwMakeContextCurrent(this->get_window());
            }
            void opengl_context::draw_indexed(size_t index_count) {
                glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, nullptr);
            }
            void opengl_context::swap_buffers() {
                glfwSwapBuffers(this->get_window());
            }
            void opengl_context::setup_glfw() {
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            }
            void opengl_context::setup_context() {
                gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                // todo: enable more features as they are needed
            }
            void opengl_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                glViewport(x, y, width, height);
            }
            void opengl_context::init_imgui_backends() {
                ImGui_ImplGlfw_InitForOpenGL(this->get_window(), true);
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
        }
    }
}
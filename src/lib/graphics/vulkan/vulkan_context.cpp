#include "bve_pch.h"
#include "vulkan_context.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
namespace bve {
    namespace graphics {
        namespace vulkan {
            vulkan_context::vulkan_context(ref<vulkan_object_factory> factory) {
                this->m_factory = factory;
            }
            vulkan_context::~vulkan_context() {
                vkDestroyInstance(this->m_instance, nullptr);
            }
            void vulkan_context::clear() {
                // todo: clear
            }
            void vulkan_context::make_current() {
                this->m_factory->m_current_context = this;
            }
            void vulkan_context::draw_indexed(size_t index_count) {
                // todo: draw
            }
            VkInstance vulkan_context::get_instance() {
                return this->m_instance;
            }
            void vulkan_context::swap_buffers() {
                // todo: swap buffers
            }
            void vulkan_context::setup_glfw() {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            }
            void vulkan_context::setup_context() {
                uint32_t extension_count = 0;
                vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
                spdlog::info("[vulkan context] vulkan extensions supported: {0}", extension_count);
                this->create_instance();
            }
            void vulkan_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                // todo: resize
            }
            void vulkan_context::init_imgui_backends() {
                ImGui_ImplGlfw_InitForVulkan(this->get_window(), true);
                ImGui_ImplVulkan_InitInfo info;
                info.Instance = this->m_instance;
                ImGui_ImplVulkan_Init(&info, nullptr);
            }
            void vulkan_context::shutdown_imgui_backends() {
                ImGui_ImplVulkan_Shutdown();
                ImGui_ImplGlfw_Shutdown();
            }
            void vulkan_context::call_imgui_backend_newframe() {
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
            }
            void vulkan_context::render_imgui_draw_data(ImDrawData* data) {
                ImGui_ImplVulkan_RenderDrawData(data, nullptr, nullptr);
            }
            void vulkan_context::create_instance() {
                std::string app_name = "basic-voxel-engine";
                uint32_t version = VK_MAKE_VERSION(0, 0, 1); // i should make this dynamic or something idk lol
                VkApplicationInfo app_info;
                memset(&app_info, 0, sizeof(VkApplicationInfo));
                app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                app_info.pApplicationName = app_name.c_str();
                app_info.applicationVersion = version;
                app_info.pEngineName = app_name.c_str();
                app_info.engineVersion = version;
                app_info.apiVersion = VK_API_VERSION_1_0;
                VkInstanceCreateInfo create_info;
                memset(&create_info, 0, sizeof(VkInstanceCreateInfo));
                create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                create_info.pApplicationInfo = &app_info;
                uint32_t glfw_extension_count = 0;
                const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
                create_info.enabledExtensionCount = glfw_extension_count;
                create_info.ppEnabledExtensionNames = glfw_extensions;
                create_info.enabledLayerCount = 0;
                if (vkCreateInstance(&create_info, nullptr, &this->m_instance) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create instance");
                }
            }
        }
    }
}
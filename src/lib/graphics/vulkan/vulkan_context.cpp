#include "bve_pch.h"
#include "vulkan_context.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "vulkan_extensions.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
                VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
                std::string message = "[vulkan context] validation layer: " + std::string(callback_data->pMessage);
                switch (severity) {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                    spdlog::debug(message);
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                    spdlog::warn(message);
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                    spdlog::error(message);
                    break;
                default:
                    spdlog::info(message);
                    break;
                }
                return VK_FALSE;
            }
            vulkan_context::vulkan_context(ref<vulkan_object_factory> factory) {
                this->m_factory = factory;
                this->m_validation_layers_enabled = false;
#ifndef NDEBUG
                this->m_validation_layers_enabled = true;
                this->m_layers.push_back("VK_LAYER_KHRONOS_validation");
                this->m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
            }
            vulkan_context::~vulkan_context() {
                if (this->m_validation_layers_enabled) {
                    _vkDestroyDebugUtilsMessengerEXT(this->m_instance, this->m_debug_messenger, nullptr);
                }
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
            VkDebugUtilsMessengerEXT vulkan_context::get_debug_messenger() {
                return this->m_debug_messenger;
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
                this->create_debug_messenger();
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
                if (!this->layers_supported()) {
                    throw std::runtime_error("[vulkan context] not all of the layers specified are supported");
                }
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
                std::vector<const char*> extensions = this->get_extensions();
                create_info.enabledExtensionCount = (uint32_t)extensions.size();
                create_info.ppEnabledExtensionNames = extensions.data();
                create_info.enabledLayerCount = (uint32_t)this->m_layers.size();
                create_info.ppEnabledLayerNames = this->m_layers.data();
                if (vkCreateInstance(&create_info, nullptr, &this->m_instance) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create instance");
                }
            }
            void vulkan_context::create_debug_messenger() {
                if (!this->m_validation_layers_enabled) {
                    return;
                }
                VkDebugUtilsMessengerCreateInfoEXT create_info;
                memset(&create_info, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
                create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                create_info.pfnUserCallback = debug_callback;
                create_info.pUserData = nullptr;
                if (_vkCreateDebugUtilsMessengerEXT(this->m_instance, &create_info, nullptr, &this->m_debug_messenger) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create debug messenger");
                }
            }
            bool vulkan_context::layers_supported() {
                uint32_t layer_count = 0;
                vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
                std::vector<VkLayerProperties> available_layers(layer_count);
                vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
                for (const char* layer_name : this->m_layers) {
                    bool layer_found = false;
                    for (const auto& layer_properties : available_layers) {
                        if (strcmp(layer_properties.layerName, layer_name) == 0) {
                            layer_found = true;
                            break;
                        }
                    }
                    if (!layer_found) {
                        return false;
                    }
                }
                return true;
            }
            std::vector<const char*> vulkan_context::get_extensions() {
                uint32_t glfw_extension_count = 0;
                const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
                std::vector<const char*> extensions;
                for (size_t i = 0; i < (size_t)glfw_extension_count; i++) {
                    extensions.push_back(glfw_extensions[i]);
                }
                for (const char* extension_name : this->m_extensions) {
                    extensions.push_back(extension_name);
                }
                return extensions;
            }
        }
    }
}
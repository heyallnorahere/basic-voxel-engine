#include "bve_pch.h"
#include "vulkan_context.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "vulkan_extensions.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            struct queue_data {
                uint32_t index;
                float priority;
            };
            struct queue_family_indices {
                std::optional<uint32_t> graphics_family;
                bool is_complete() {
                    return this->graphics_family.has_value();
                }
                std::vector<queue_data> get_queues() {
                    return {
                        { *this->graphics_family, 1.f }
                    };
                }
            };
            static queue_family_indices find_queue_families(VkPhysicalDevice device) {
                queue_family_indices indices;
                uint32_t queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
                for (uint32_t i = 0; i < queue_family_count; i++) {
                    if (indices.is_complete()) {
                        break;
                    }
                    VkQueueFamilyProperties queue_family = queue_families[i];
                    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        indices.graphics_family = i;
                    }
                }
                return indices;
            }
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
                vkDestroyDevice(this->m_device, nullptr);
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
            VkPhysicalDevice vulkan_context::get_physical_device() {
                return this->m_physical_device;
            }
            VkDevice vulkan_context::get_device() {
                return this->m_device;
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
                this->pick_physical_device();
                this->create_logical_device();
            }
            void vulkan_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                // todo: resize
            }
            void vulkan_context::init_imgui_backends() {
                ImGui_ImplGlfw_InitForVulkan(this->get_window(), true);
                ImGui_ImplVulkan_InitInfo info;
                info.Instance = this->m_instance;
                info.PhysicalDevice = this->m_physical_device;
                info.Device = this->m_device;
                info.QueueFamily = *find_queue_families(this->m_physical_device).graphics_family;
                info.Queue = this->m_graphics_queue;
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
            void vulkan_context::pick_physical_device() {
                this->m_physical_device = nullptr;
                uint32_t device_count = 0;
                vkEnumeratePhysicalDevices(this->m_instance, &device_count, nullptr);
                if (device_count == 0) {
                    throw std::runtime_error("[vulkan context] no GPUs installed in this system have Vulkan support");
                }
                std::vector<VkPhysicalDevice> devices(device_count);
                vkEnumeratePhysicalDevices(this->m_instance, &device_count, devices.data());
                std::multimap<uint32_t, VkPhysicalDevice> candidates;
                for (const auto& device : devices) {
                    uint32_t score = this->rate_device(device);
                    candidates.insert({ score, device });
                }
                auto it = candidates.rbegin();
                if (it->first > 0) {
                    this->m_physical_device = it->second;
                } else {
                    throw std::runtime_error("[vulkan context] could not find a suitable GPU");
                }
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(this->m_physical_device, &properties);
                spdlog::info("[vulkan context] picked physical device: {0}", properties.deviceName);
            }
            void vulkan_context::create_logical_device() {
                auto indices = find_queue_families(this->m_physical_device);
                std::vector<queue_data> queues = indices.get_queues();
                std::vector<VkDeviceQueueCreateInfo> queue_create_info;
                for (const auto& queue : queues) {
                    VkDeviceQueueCreateInfo create_info;
                    memset(&create_info, 0, sizeof(VkDeviceQueueCreateInfo));
                    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    create_info.queueFamilyIndex = queue.index;
                    create_info.queueCount = 1;
                    float queue_priority = queue.priority;
                    create_info.pQueuePriorities = &queue_priority;
                    queue_create_info.push_back(create_info);
                }
                VkDeviceCreateInfo create_info;
                memset(&create_info, 0, sizeof(VkDeviceCreateInfo));
                create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                create_info.pQueueCreateInfos = queue_create_info.data();
                create_info.queueCreateInfoCount = (uint32_t)queue_create_info.size();
                VkPhysicalDeviceFeatures features;
                memset(&features, 0, sizeof(VkPhysicalDeviceFeatures));
                create_info.pEnabledFeatures = &features;
                if (vkCreateDevice(this->m_physical_device, &create_info, nullptr, &this->m_device) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create logical device");
                }
                vkGetDeviceQueue(this->m_device, *indices.graphics_family, 0, &this->m_graphics_queue);
            }
            uint32_t vulkan_context::rate_device(VkPhysicalDevice device) {
                VkPhysicalDeviceProperties properties;
                VkPhysicalDeviceFeatures features;
                vkGetPhysicalDeviceProperties(device, &properties);
                vkGetPhysicalDeviceFeatures(device, &features);
                uint32_t score = 0;
                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    score += 1000;
                }
                score += properties.limits.maxImageDimension2D;
                if (!features.geometryShader) {
                    return 0;
                }
                if (!find_queue_families(device).is_complete()) {
                    return 0;
                }
                return score;
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
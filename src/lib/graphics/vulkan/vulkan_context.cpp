#include "bve_pch.h"
#include "vulkan_context.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "vulkan_extensions.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            struct swap_chain_support_details {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> present_modes;
            };
            static swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR window_surface) {
                swap_chain_support_details details;
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window_surface, &details.capabilities);
                uint32_t format_count = 0;
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, window_surface, &format_count, nullptr);
                if (format_count > 0) {
                    details.formats.resize(format_count);
                    vkGetPhysicalDeviceSurfaceFormatsKHR(device, window_surface, &format_count, details.formats.data());
                }
                uint32_t present_mode_count = 0;
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, window_surface, &present_mode_count, nullptr);
                if (present_mode_count > 0) {
                    details.present_modes.resize(present_mode_count);
                    vkGetPhysicalDeviceSurfacePresentModesKHR(device, window_surface, &present_mode_count, details.present_modes.data());
                }
                return details;
            }
            static VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
                for (const auto& format : available_formats) {
                    if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                        return format;
                    }
                }
                return available_formats[0];
            }
            static VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
                for (const auto& present_mode : available_present_modes) {
                    if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                        return present_mode;
                    }
                }
                return VK_PRESENT_MODE_FIFO_KHR;
            }
            static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, glm::ivec2 framebuffer_size) {
                if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                    return capabilities.currentExtent;
                } else {
                    VkExtent2D extent;
                    extent.width = std::clamp((uint32_t)framebuffer_size.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                    extent.height = std::clamp((uint32_t)framebuffer_size.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
                    return extent;
                }
            }
            struct queue_family_indices {
                std::optional<uint32_t> graphics_family, present_family;
                bool is_complete() {
                    return this->graphics_family.has_value() && this->present_family.has_value();
                }
            };
            static queue_family_indices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR window_surface) {
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
                    VkBool32 present_support = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window_surface, &present_support);
                    if (present_support) {
                        indices.present_family = i;
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
                this->m_device_extensions = {
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME
                };
#ifndef NDEBUG
                this->m_validation_layers_enabled = true;
                this->m_layers.push_back("VK_LAYER_KHRONOS_validation");
                this->m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
            }
            vulkan_context::~vulkan_context() {
                for (const auto& image_view : this->m_swapchain_image_views) {
                    vkDestroyImageView(this->m_device, image_view, nullptr);
                }
                vkDestroySwapchainKHR(this->m_device, this->m_swap_chain, nullptr);
                vkDestroyDevice(this->m_device, nullptr);
                if (this->m_validation_layers_enabled) {
                    _vkDestroyDebugUtilsMessengerEXT(this->m_instance, this->m_debug_messenger, nullptr);
                }
                vkDestroySurfaceKHR(this->m_instance, this->m_window_surface, nullptr);
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
            VkQueue vulkan_context::get_graphics_queue() {
                return this->m_graphics_queue;
            }
            VkSurfaceKHR vulkan_context::get_window_surface() {
                return this->m_window_surface;
            }
            VkSwapchainKHR vulkan_context::get_swap_chain() {
                return this->m_swap_chain;
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
                this->create_window_surface();
                this->pick_physical_device();
                this->create_logical_device();
                glm::ivec2 window_size;
                glfwGetFramebufferSize(this->m_window, &window_size.x, &window_size.y);
                this->m_swap_chain = nullptr;
                this->create_swap_chain(window_size);
                this->create_image_views();
            }
            void vulkan_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                // todo: resize
            }
            void vulkan_context::init_imgui_backends() {
                // broken atm - not enough stuff implemented
                /*
                ImGui_ImplGlfw_InitForVulkan(this->m_window, true);
                ImGui_ImplVulkan_InitInfo info;
                info.Instance = this->m_instance;
                info.PhysicalDevice = this->m_physical_device;
                info.Device = this->m_device;
                info.QueueFamily = *find_queue_families(this->m_physical_device, this->m_window_surface).graphics_family;
                info.Queue = this->m_graphics_queue;
                info.ImageCount = this->m_image_count;
                info.MinImageCount = this->m_min_image_count;
                
                ImGui_ImplVulkan_Init(&info, nullptr);
                */
            }
            void vulkan_context::shutdown_imgui_backends() {
                /*
                ImGui_ImplVulkan_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                */
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
            void vulkan_context::create_window_surface() {
                if (glfwCreateWindowSurface(this->m_instance, this->m_window, nullptr, &this->m_window_surface) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create window surface");
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
                auto indices = find_queue_families(this->m_physical_device, this->m_window_surface);
                std::set<uint32_t> queue_families = { *indices.graphics_family, *indices.present_family };
                std::vector<VkDeviceQueueCreateInfo> queue_create_info;
                float queue_priority = 1.f;
                for (uint32_t index : queue_families) {
                    VkDeviceQueueCreateInfo create_info;
                    memset(&create_info, 0, sizeof(VkDeviceQueueCreateInfo));
                    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    create_info.queueFamilyIndex = index;
                    create_info.queueCount = 1;
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
                create_info.enabledExtensionCount = (uint32_t)this->m_device_extensions.size();
                create_info.ppEnabledExtensionNames = this->m_device_extensions.data();
                if (vkCreateDevice(this->m_physical_device, &create_info, nullptr, &this->m_device) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create logical device");
                }
                vkGetDeviceQueue(this->m_device, *indices.graphics_family, 0, &this->m_graphics_queue);
                vkGetDeviceQueue(this->m_device, *indices.present_family, 0, &this->m_present_queue);
            }
            void vulkan_context::create_swap_chain(glm::ivec2 size) {
                auto details = query_swap_chain_support(this->m_physical_device, this->m_window_surface);
                auto format = choose_swap_surface_format(details.formats);
                auto present_mode = choose_swap_present_mode(details.present_modes);
                auto extent = choose_swap_extent(details.capabilities, size);
                this->m_min_image_count = details.capabilities.minImageCount;
                this->m_image_count = this->m_min_image_count + 1;
                if (details.capabilities.maxImageCount > 0 && this->m_image_count > details.capabilities.maxImageCount) {
                    this->m_image_count = details.capabilities.maxImageCount;
                }
                VkSwapchainCreateInfoKHR create_info;
                memset(&create_info, 0, sizeof(VkSwapchainCreateInfoKHR));
                create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                create_info.surface = this->m_window_surface;
                create_info.minImageCount = this->m_image_count;
                create_info.imageFormat = format.format;
                create_info.imageColorSpace = format.colorSpace;
                create_info.imageExtent = extent;
                create_info.imageArrayLayers = 1;
                create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                auto indices = find_queue_families(this->m_physical_device, this->m_window_surface);
                std::vector<uint32_t> queue_families = { *indices.graphics_family, *indices.present_family };
                if (indices.graphics_family != indices.present_family) {
                    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                    create_info.queueFamilyIndexCount = (uint32_t)queue_families.size();
                    create_info.pQueueFamilyIndices = queue_families.data();
                } else {
                    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                    create_info.queueFamilyIndexCount = 0;
                    create_info.pQueueFamilyIndices = nullptr;
                }
                create_info.preTransform = details.capabilities.currentTransform;
                create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                create_info.presentMode = present_mode;
                create_info.clipped = true;
                create_info.oldSwapchain = this->m_swap_chain;
                if (vkCreateSwapchainKHR(this->m_device, &create_info, nullptr, &this->m_swap_chain) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create swapchain");
                }
                this->m_swapchain_image_format = format.format;
                this->m_swapchain_extent = extent;
                vkGetSwapchainImagesKHR(this->m_device, this->m_swap_chain, &this->m_image_count, nullptr);
                this->m_swapchain_images.resize(this->m_image_count);
                vkGetSwapchainImagesKHR(this->m_device, this->m_swap_chain, &this->m_image_count, this->m_swapchain_images.data());
            }
            void vulkan_context::create_image_views() {
                for (const auto& view : this->m_swapchain_image_views) {
                    vkDestroyImageView(this->m_device, view, nullptr);
                }
                this->m_swapchain_image_views.clear();
                for (size_t i = 0; i < this->m_swapchain_images.size(); i++) {
                    VkImageViewCreateInfo create_info;
                    memset(&create_info, 0, sizeof(VkImageViewCreateInfo));
                    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    create_info.image = this->m_swapchain_images[i];
                    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    create_info.format = this->m_swapchain_image_format;
                    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    create_info.subresourceRange.baseMipLevel = 0;
                    create_info.subresourceRange.levelCount = 1;
                    create_info.subresourceRange.baseArrayLayer = 0;
                    create_info.subresourceRange.layerCount = 1;
                    VkImageView image_view;
                    if (vkCreateImageView(this->m_device, &create_info, nullptr, &image_view) != VK_SUCCESS) {
                        throw std::runtime_error("[vulkan context] could not create image view " + std::to_string(i));
                    }
                }
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
                bool extensions_supported = this->check_device_extension_support(device);
                bool swap_chain_adequate = false;
                if (extensions_supported) {
                    auto details = query_swap_chain_support(device, this->m_window_surface);
                    swap_chain_adequate = !details.formats.empty() && !details.present_modes.empty();
                }
                bool device_suitable = features.geometryShader && find_queue_families(device, this->m_window_surface).is_complete() && extensions_supported && swap_chain_adequate;
                if (!device_suitable) {
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
            bool vulkan_context::check_device_extension_support(VkPhysicalDevice device) {
                uint32_t extension_count = 0;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
                std::vector<VkExtensionProperties> available_extensions(extension_count);
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());
                std::set<std::string> required_extensions(this->m_device_extensions.begin(), this->m_device_extensions.end());
                for (const auto& extension : available_extensions) {
                    required_extensions.erase(extension.extensionName);
                }
                return required_extensions.empty();
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
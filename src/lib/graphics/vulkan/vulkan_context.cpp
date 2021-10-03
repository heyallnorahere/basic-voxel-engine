#include "bve_pch.h"
#include "vulkan_context.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "vulkan_extensions.h"
#include "vulkan_pipeline.h"
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
                this->m_current_command_buffer = 0;
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
                vkDestroySemaphore(this->m_device, this->m_render_finished_semaphore, nullptr);
                vkDestroySemaphore(this->m_device, this->m_image_available_semaphore, nullptr);
                this->cleanup_swapchain(nullptr);
                vkDestroyCommandPool(this->m_device, this->m_command_pool, nullptr);
                vkDestroyDevice(this->m_device, nullptr);
                if (this->m_validation_layers_enabled) {
                    _vkDestroyDebugUtilsMessengerEXT(this->m_instance, this->m_debug_messenger, nullptr);
                }
                vkDestroySurfaceKHR(this->m_instance, this->m_window_surface, nullptr);
                vkDestroyInstance(this->m_instance, nullptr);
            }
            void vulkan_context::clear(glm::vec4 clear_color) {
                vkAcquireNextImageKHR(this->m_device, this->m_swap_chain, UINT64_MAX, this->m_image_available_semaphore, nullptr, &this->m_current_command_buffer);
                VkCommandBuffer command_buffer = this->m_command_buffers[this->m_current_command_buffer];
                VkCommandBufferBeginInfo begin_info;
                memset(&begin_info, 0, sizeof(VkCommandBufferBeginInfo));
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                begin_info.flags = 0;
                begin_info.pInheritanceInfo = nullptr;
                if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not begin recording a command buffer");
                }
                VkRenderPassBeginInfo render_pass_info;
                memset(&render_pass_info, 0, sizeof(VkRenderPassBeginInfo));
                render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_info.renderPass = this->m_render_pass;
                render_pass_info.framebuffer = this->m_framebuffers[this->m_current_command_buffer];
                render_pass_info.renderArea.offset = { 0, 0 };
                render_pass_info.renderArea.extent = this->m_swapchain_extent;
                render_pass_info.clearValueCount = 1;
                render_pass_info.pClearValues = (VkClearValue*)&clear_color;
                vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            }
            void vulkan_context::make_current() {
                this->m_factory->m_current_context = this;
            }
            void vulkan_context::draw_indexed(size_t index_count) {
                VkCommandBuffer command_buffer = this->m_command_buffers[this->m_current_command_buffer];
                auto pipeline = this->m_factory->m_current_pipeline;
                if (pipeline) {
                    auto vk_pipeline = pipeline.as<vulkan_pipeline>();
                    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline->get_pipeline());
                } else {
                    throw std::runtime_error("[vulkan context] a pipeline must be bound in order to render");
                }
                vkCmdDrawIndexed(command_buffer, (uint32_t)index_count, 1, 0, 0, 0);
            }
            void vulkan_context::swap_buffers() {
                VkCommandBuffer command_buffer = this->m_command_buffers[this->m_current_command_buffer];
                vkCmdEndRenderPass(command_buffer);
                if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not finish recording a command buffer");
                }
                VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
                VkSubmitInfo submit_info;
                memset(&submit_info, 0, sizeof(VkSubmitInfo));
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.waitSemaphoreCount = 1;
                submit_info.pWaitSemaphores = &this->m_image_available_semaphore;
                submit_info.pWaitDstStageMask = wait_stages;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &command_buffer;
                submit_info.signalSemaphoreCount = 1;
                submit_info.pSignalSemaphores = &this->m_render_finished_semaphore;
                if (vkQueueSubmit(this->m_graphics_queue, 1, &submit_info, nullptr) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not submit command buffer");
                }
                VkPresentInfoKHR present_info;
                memset(&present_info, 0, sizeof(VkPresentInfoKHR));
                present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                present_info.waitSemaphoreCount = 1;
                present_info.pWaitSemaphores = &this->m_render_finished_semaphore;
                present_info.swapchainCount = 1;
                present_info.pSwapchains = &this->m_swap_chain;
                present_info.pImageIndices = &this->m_current_command_buffer;
                present_info.pResults = nullptr;
                if (vkQueuePresentKHR(this->m_present_queue, &present_info) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not present");
                }
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
                this->create_render_pass();
                this->create_framebuffers();
                this->create_command_pool();
                this->alloc_command_buffers();
                this->create_semaphores();            }
            void vulkan_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                vkDeviceWaitIdle(this->m_device);
                bool recreate_pipeline;
                this->cleanup_swapchain(&recreate_pipeline);
                this->create_swap_chain(glm::ivec2(x, y));
                this->create_image_views();
                this->create_render_pass();
                if (recreate_pipeline) {
                    auto pipeline = this->m_factory->m_current_pipeline.as<vulkan_pipeline>();
                    pipeline->create();
                }
                this->create_framebuffers();
                this->alloc_command_buffers();
            }
            void vulkan_context::init_imgui_backends() {
                ImGui_ImplGlfw_InitForVulkan(this->m_window, true);
                ImGui_ImplVulkan_InitInfo info;
                memset(&info, 0, sizeof(ImGui_ImplVulkan_InitInfo));
                info.Instance = this->m_instance;
                info.PhysicalDevice = this->m_physical_device;
                info.Device = this->m_device;
                info.QueueFamily = *find_queue_families(this->m_physical_device, this->m_window_surface).graphics_family;
                info.Queue = this->m_graphics_queue;
                info.ImageCount = this->m_image_count;
                info.MinImageCount = this->m_min_image_count;
                ImGui_ImplVulkan_Init(&info, this->m_render_pass);
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
                ImGui_ImplVulkan_RenderDrawData(data, this->m_command_buffers[this->m_current_command_buffer], nullptr);
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
                    this->m_swapchain_image_views.push_back(image_view);
                }
            }
            void vulkan_context::create_render_pass() {
                VkAttachmentDescription color_attachment;
                memset(&color_attachment, 0, sizeof(VkAttachmentDescription));
                color_attachment.format = this->m_swapchain_image_format;
                color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                VkAttachmentReference color_attachment_ref;
                memset(&color_attachment_ref, 0, sizeof(VkAttachmentReference));
                color_attachment_ref.attachment = 0;
                color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                VkSubpassDescription subpass;
                memset(&subpass, 0, sizeof(VkSubpassDescription));
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments = &color_attachment_ref;
                VkSubpassDependency dependency;
                memset(&dependency, 0, sizeof(VkSubpassDependency));
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = 0;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                VkRenderPassCreateInfo create_info;
                memset(&create_info, 0, sizeof(VkRenderPassCreateInfo));
                create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                create_info.attachmentCount = 1;
                create_info.pAttachments = &color_attachment;
                create_info.subpassCount = 1;
                create_info.pSubpasses = &subpass;
                create_info.dependencyCount = 1;
                create_info.pDependencies = &dependency;
                if (vkCreateRenderPass(this->m_device, &create_info, nullptr, &this->m_render_pass) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create render pass");
                }
            }
            void vulkan_context::create_framebuffers() {
                this->m_framebuffers.resize(this->m_swapchain_image_views.size());
                for (size_t i = 0; i < this->m_swapchain_image_views.size(); i++) {
                    VkFramebufferCreateInfo create_info;
                    memset(&create_info, 0, sizeof(VkFramebufferCreateInfo));
                    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    create_info.renderPass = this->m_render_pass;
                    create_info.attachmentCount = 1;
                    create_info.pAttachments = &this->m_swapchain_image_views[i];
                    create_info.width = this->m_swapchain_extent.width;
                    create_info.height = this->m_swapchain_extent.height;
                    create_info.layers = 1;
                    if (vkCreateFramebuffer(this->m_device, &create_info, nullptr, &this->m_framebuffers[i]) != VK_SUCCESS) {
                        throw std::runtime_error("[vulkan context] could not create framebuffer");
                    }
                }
            }
            void vulkan_context::create_command_pool() {
                auto indices = find_queue_families(this->m_physical_device, this->m_window_surface);
                VkCommandPoolCreateInfo create_info;
                memset(&create_info, 0, sizeof(VkCommandPoolCreateInfo));
                create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                create_info.queueFamilyIndex = *indices.graphics_family;
                create_info.flags = 0;
                if (vkCreateCommandPool(this->m_device, &create_info, nullptr, &this->m_command_pool) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create command pool");
                }
            }
            void vulkan_context::alloc_command_buffers() {
                this->m_command_buffers.resize(this->m_framebuffers.size());
                VkCommandBufferAllocateInfo alloc_info;
                memset(&alloc_info, 0, sizeof(VkCommandBufferAllocateInfo));
                alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                alloc_info.commandPool = this->m_command_pool;
                alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                alloc_info.commandBufferCount = (uint32_t)this->m_command_buffers.size();
                if (vkAllocateCommandBuffers(this->m_device, &alloc_info, this->m_command_buffers.data()) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not allocate command buffers");
                }
            }
            void vulkan_context::create_semaphores() {
                VkSemaphoreCreateInfo create_info;
                memset(&create_info, 0, sizeof(VkSemaphoreCreateInfo));
                create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                if (vkCreateSemaphore(this->m_device, &create_info, nullptr, &this->m_image_available_semaphore) != VK_SUCCESS ||
                    vkCreateSemaphore(this->m_device, &create_info, nullptr, &this->m_render_finished_semaphore) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create semaphores");
                }
            }
            void vulkan_context::cleanup_swapchain(bool* recreate_pipeline) {
                vkFreeCommandBuffers(this->m_device, this->m_command_pool, (uint32_t)this->m_command_buffers.size(), this->m_command_buffers.data());
                this->m_command_buffers.clear();
                for (VkFramebuffer framebuffer : this->m_framebuffers) {
                    vkDestroyFramebuffer(this->m_device, framebuffer, nullptr);
                }
                this->m_framebuffers.clear();
                if (recreate_pipeline && this->m_factory->m_current_pipeline) {
                    auto pipeline = this->m_factory->m_current_pipeline.as<vulkan_pipeline>();
                    *recreate_pipeline = false;
                    if (pipeline->valid()) {
                        pipeline->destroy();
                        *recreate_pipeline = true;
                    }
                }
                vkDestroyRenderPass(this->m_device, this->m_render_pass, nullptr);
                for (VkImageView image_view : this->m_swapchain_image_views) {
                    vkDestroyImageView(this->m_device, image_view, nullptr);
                }
                this->m_swapchain_image_views.clear();
                vkDestroySwapchainKHR(this->m_device, this->m_swap_chain, nullptr);
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
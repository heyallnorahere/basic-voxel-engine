#include "bve_pch.h"
#include "vulkan_context.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "vulkan_extensions.h"
#include "vulkan_pipeline.h"
#include "vulkan_uniform_buffer.h"
#include "vulkan_texture.h"
#include "util.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            constexpr size_t max_frames_in_flight = 2;
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
                this->m_resize_swapchain = false;
                this->m_current_frame = 0;
                this->m_current_image = 0;
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
                for (size_t i = 0; i < max_frames_in_flight; i++) {
                    vkDestroySemaphore(this->m_device, this->m_render_finished_semaphores[i], nullptr);
                    vkDestroySemaphore(this->m_device, this->m_image_available_semaphores[i], nullptr);
                }
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
                VkCommandBufferBeginInfo begin_info;
                util::zero(begin_info);
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                begin_info.flags = 0;
                begin_info.pInheritanceInfo = nullptr;
                if (vkBeginCommandBuffer(this->m_command_buffers[this->m_current_image], &begin_info) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not begin recording a command buffer");
                }
                VkRenderPassBeginInfo render_pass_info;
                util::zero(render_pass_info);
                render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_info.renderPass = this->m_render_pass;
                render_pass_info.framebuffer = this->m_framebuffers[this->m_current_image];
                render_pass_info.renderArea.offset = { 0, 0 };
                render_pass_info.renderArea.extent = this->m_swapchain_extent;
                std::array<VkClearValue, 2> clear_values;
                util::zero(clear_values.data(), clear_values.size() * sizeof(VkClearValue));
                memcpy(clear_values[0].color.float32, &clear_color, sizeof(glm::vec4));
                clear_values[1].depthStencil = { 1.f, 0 };
                render_pass_info.clearValueCount = (uint32_t)clear_values.size();
                render_pass_info.pClearValues = clear_values.data();
                vkCmdBeginRenderPass(this->m_command_buffers[this->m_current_image], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            }
            void vulkan_context::make_current() {
                this->m_factory->m_current_context = this;
            }
            void vulkan_context::draw_indexed(size_t index_count) {
                auto pipeline = this->m_factory->m_current_pipeline;
                if (pipeline) {
                    auto vk_pipeline = pipeline.as<vulkan_pipeline>();
                    if (!vk_pipeline->valid()) {
                        vk_pipeline->create();
                    }
                    this->m_bound_pipelines.push_back(pipeline);
                    auto shader = vk_pipeline->get_shader();
                    shader->update_descriptor_sets(this->m_current_image);
                    const auto& bound_buffers = vk_pipeline->get_bound_buffers();
                    bool bound_buffer = false;
                    if (bound_buffers.find(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) != bound_buffers.end()) {
                        const auto& bound_vertex_buffers = bound_buffers.find(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)->second;
                        if (!bound_vertex_buffers.empty()) {
                            std::vector<VkBuffer> vk_buffers;
                            for (auto buffer : bound_vertex_buffers) {
                                vk_buffers.push_back(buffer->get_buffer());
                            }
                            std::vector<VkDeviceSize> offsets(vk_buffers.size(), 0);
                            vkCmdBindVertexBuffers(this->m_command_buffers[this->m_current_image], 0, (uint32_t)vk_buffers.size(), vk_buffers.data(), offsets.data());
                            bound_buffer = true;
                        }
                    }
                    if (!bound_buffer) {
                        spdlog::warn("[vulkan context] attempting to call vkCmdDrawIndexed without a vertex buffer");
                    }
                    bound_buffer = false;
                    if (bound_buffers.find(VK_BUFFER_USAGE_INDEX_BUFFER_BIT) != bound_buffers.end()) {
                        const auto& bound_index_buffers = bound_buffers.find(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)->second;
                        if (!bound_index_buffers.empty()) {
                            auto front = *bound_index_buffers.begin();
                            vkCmdBindIndexBuffer(this->m_command_buffers[this->m_current_image], front->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
                            bound_buffer = true;
                        }
                    }
                    if (!bound_buffer) {
                        spdlog::warn("[vulkan context] attempting to call vkCmdDrawIndexed without an index buffer");
                    }
                    vkCmdBindPipeline(this->m_command_buffers[this->m_current_image], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline->get_pipeline());
                    const auto& sets = shader->get_descriptor_sets();
                    std::vector<VkDescriptorSet> sets_to_bind;
                    for (const auto& set : sets) {
                        sets_to_bind.push_back(set.sets[this->m_current_image]);
                    }
                    VkPipelineLayout layout = vk_pipeline->get_layout();
                    vkCmdBindDescriptorSets(this->m_command_buffers[this->m_current_image], VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, (uint32_t)sets_to_bind.size(), sets_to_bind.data(), 0, nullptr);
                } else {
                    throw std::runtime_error("[vulkan context] a pipeline must be bound in order to render");
                }
                vkCmdDrawIndexed(this->m_command_buffers[this->m_current_image], (uint32_t)index_count, 1, 0, 0, 0);
            }
            VkCommandBuffer vulkan_context::begin_single_time_commands() {
                VkCommandBufferAllocateInfo alloc_info;
                util::zero(alloc_info);
                alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                alloc_info.commandPool = this->m_command_pool;
                alloc_info.commandBufferCount = 1;
                VkCommandBuffer command_buffer;
                if (vkAllocateCommandBuffers(this->m_device, &alloc_info, &command_buffer) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not allocate single-time command buffer");
                }
                VkCommandBufferBeginInfo begin_info;
                util::zero(begin_info);
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not begin single-time command buffer");
                }
                return command_buffer;
            }
            void vulkan_context::end_single_time_commands(VkCommandBuffer command_buffer) {
                if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan buffer] could not end recording of copy command");
                }
                VkSubmitInfo submit_info;
                util::zero(submit_info);
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &command_buffer;
                VkFenceCreateInfo fence_info;
                util::zero(fence_info);
                fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                VkFence fence;
                if (vkCreateFence(this->m_device, &fence_info, nullptr, &fence) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan buffer] could not create fence for copying buffers");
                }
                vkQueueSubmit(this->m_graphics_queue, 1, &submit_info, fence);
                vkWaitForFences(this->m_device, 1, &fence, true, UINT64_MAX);
                vkDestroyFence(this->m_device, fence, nullptr);
                vkFreeCommandBuffers(this->m_device, this->m_command_pool, 1, &command_buffer);
            }
            void vulkan_context::swap_buffers() {
                auto resize_swapchain = [this]() mutable {
                    int32_t width = 0, height = 0;
                    glfwGetFramebufferSize(this->m_window, &width, &height);
                    if (width == 0 || height == 0) {
                        // why do we do this again?
                        glfwGetFramebufferSize(this->m_window, &width, &height);
                        glfwWaitEvents();
                    }
                    this->recreate_swapchain(glm::ivec2(width, height));
                };
                vkCmdEndRenderPass(this->m_command_buffers[this->m_current_image]);
                if (vkEndCommandBuffer(this->m_command_buffers[this->m_current_image]) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not finish recording a command buffer");
                }
                VkFenceCreateInfo fence_info;
                util::zero(fence_info);
                fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                VkFence fence;
                if (vkCreateFence(this->m_device, &fence_info, nullptr, &fence) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create fence for rendering");
                }
                uint32_t current_image;
                VkResult result = vkAcquireNextImageKHR(this->m_device, this->m_swap_chain, UINT64_MAX, this->m_image_available_semaphores[this->m_current_frame], fence, &current_image);
                if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                    resize_swapchain();
                } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                    throw std::runtime_error("[vulkan context] could not acquire next swapchain image");
                }
                vkWaitForFences(this->m_device, 1, &fence, true, UINT64_MAX);
                vkResetFences(this->m_device, 1, &fence);
                VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
                VkSubmitInfo submit_info;
                util::zero(submit_info);
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.waitSemaphoreCount = 1;
                submit_info.pWaitSemaphores = &this->m_image_available_semaphores[this->m_current_frame];
                submit_info.pWaitDstStageMask = wait_stages;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &this->m_command_buffers[current_image];
                submit_info.signalSemaphoreCount = 1;
                submit_info.pSignalSemaphores = &this->m_render_finished_semaphores[this->m_current_frame];
                if (vkQueueSubmit(this->m_graphics_queue, 1, &submit_info, fence) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not submit command buffer");
                }
                if (vkWaitForFences(this->m_device, 1, &fence, true, UINT64_MAX) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] an error occurred waiting for the render calls to finish");
                }
                vkDestroyFence(this->m_device, fence, nullptr);
                VkPresentInfoKHR present_info;
                util::zero(present_info);
                present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                present_info.waitSemaphoreCount = 1;
                present_info.pWaitSemaphores = &this->m_render_finished_semaphores[this->m_current_frame];
                present_info.swapchainCount = 1;
                present_info.pSwapchains = &this->m_swap_chain;
                present_info.pImageIndices = &current_image;
                present_info.pResults = nullptr;
                result = vkQueuePresentKHR(this->m_present_queue, &present_info);
                if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->m_resize_swapchain) {
                    this->m_resize_swapchain = false;
                    resize_swapchain();
                } else if (result != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not present");
                }
                this->m_current_frame = (this->m_current_frame + 1) % max_frames_in_flight;
                this->m_current_image = (current_image + 1) % (uint32_t)this->m_swapchain_images.size();
                this->m_bound_pipelines.clear();
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
                this->create_command_pool();
                this->create_depth_resources();
                this->create_framebuffers();
                this->alloc_command_buffers();
                this->create_descriptor_pool();
                this->create_sync_objects();
            }
            void vulkan_context::resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
                this->m_resize_swapchain = true;
            }
            void vulkan_context::init_imgui_backends() {
                ImGui_ImplGlfw_InitForVulkan(this->m_window, true);
                ImGui_ImplVulkan_InitInfo info;
                util::zero(info);
                info.Instance = this->m_instance;
                info.PhysicalDevice = this->m_physical_device;
                info.Device = this->m_device;
                info.QueueFamily = *find_queue_families(this->m_physical_device, this->m_window_surface).graphics_family;
                info.Queue = this->m_graphics_queue;
                info.ImageCount = this->m_image_count;
                info.MinImageCount = this->m_min_image_count;
                info.DescriptorPool = this->m_descriptor_pool;
                ImGui_ImplVulkan_Init(&info, this->m_render_pass);
            }
            void vulkan_context::shutdown_imgui_backends() {
                ImGui_ImplVulkan_Shutdown();
                ImGui_ImplGlfw_Shutdown();
            }
            void vulkan_context::call_imgui_backend_newframe() {
                static bool first_frame = true;
                if (first_frame) {
                    VkCommandBuffer command_buffer = this->begin_single_time_commands();
                    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
                    this->end_single_time_commands(command_buffer);
                    ImGui_ImplVulkan_DestroyFontUploadObjects();
                    first_frame = false;
                }
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
            }
            void vulkan_context::render_imgui_draw_data(ImDrawData* data) {
                ImGui_ImplVulkan_RenderDrawData(data, this->m_command_buffers[this->m_current_image]);
            }
            void vulkan_context::create_instance() {
                if (!this->layers_supported()) {
                    throw std::runtime_error("[vulkan context] not all of the layers specified are supported");
                }
                uint32_t version = VK_MAKE_VERSION(0, 0, 1); // i should make this dynamic or something idk lol
                VkApplicationInfo app_info;
                util::zero(app_info);
                app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                app_info.pApplicationName = "basic voxel engine";
                app_info.applicationVersion = version;
                app_info.pEngineName = "raw fucking c++";
                app_info.engineVersion = version;
                app_info.apiVersion = VK_API_VERSION_1_0;
                VkInstanceCreateInfo create_info;
                util::zero(create_info);
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
                util::zero(create_info);
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
                    util::zero(create_info);
                    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    create_info.queueFamilyIndex = index;
                    create_info.queueCount = 1;
                    create_info.pQueuePriorities = &queue_priority;
                    queue_create_info.push_back(create_info);
                }
                VkDeviceCreateInfo create_info;
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                create_info.pQueueCreateInfos = queue_create_info.data();
                create_info.queueCreateInfoCount = (uint32_t)queue_create_info.size();
                VkPhysicalDeviceFeatures features;
                util::zero(features);
                features.samplerAnisotropy = true;
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
                util::zero(create_info);
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
                    auto image_view = create_image_view(this->m_swapchain_images[i], this->m_swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, this->m_device);
                    this->m_swapchain_image_views.push_back(image_view);
                }
            }
            static VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physical_device) {
                for (VkFormat format : candidates) {
                    VkFormatProperties properties;
                    vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);
                    if ((tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) ||
                        (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)) {
                        return format;
                    }
                }
                throw std::runtime_error("[vulkan context] could not find a supported format");
                return VK_FORMAT_UNDEFINED;
            }
            static VkFormat find_depth_format(VkPhysicalDevice physical_device) {
                return find_supported_format({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, physical_device);
            }
            void vulkan_context::create_render_pass() {
                VkAttachmentDescription color_attachment;
                util::zero(color_attachment);
                color_attachment.format = this->m_swapchain_image_format;
                color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                VkAttachmentDescription depth_attachment;
                util::zero(depth_attachment);
                depth_attachment.format = find_depth_format(this->m_physical_device);
                depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                VkAttachmentReference color_attachment_ref;
                util::zero(color_attachment_ref);
                color_attachment_ref.attachment = 0;
                color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                VkAttachmentReference depth_attachment_ref;
                util::zero(depth_attachment_ref);
                depth_attachment_ref.attachment = 1;
                depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                VkSubpassDescription subpass;
                util::zero(subpass);
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments = &color_attachment_ref;
                subpass.pDepthStencilAttachment = &depth_attachment_ref;
                VkSubpassDependency dependency;
                util::zero(dependency);
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                dependency.srcAccessMask = 0;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                std::vector<VkAttachmentDescription> attachments = { color_attachment, depth_attachment };
                VkRenderPassCreateInfo create_info;
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                create_info.attachmentCount = (uint32_t)attachments.size();
                create_info.pAttachments = attachments.data();
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
                    std::vector<VkImageView> attachments = { this->m_swapchain_image_views[i], this->m_depth_image_view };
                    VkFramebufferCreateInfo create_info;
                    util::zero(create_info);
                    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    create_info.renderPass = this->m_render_pass;
                    create_info.attachmentCount = (uint32_t)attachments.size();
                    create_info.pAttachments = attachments.data();
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
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                create_info.queueFamilyIndex = *indices.graphics_family;
                create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                if (vkCreateCommandPool(this->m_device, &create_info, nullptr, &this->m_command_pool) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create command pool");
                }
            }
            void vulkan_context::alloc_command_buffers() {
                this->m_command_buffers.resize(this->m_swapchain_images.size());
                VkCommandBufferAllocateInfo alloc_info;
                util::zero(alloc_info);
                alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                alloc_info.commandPool = this->m_command_pool;
                alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                alloc_info.commandBufferCount = (uint32_t)this->m_command_buffers.size();
                if (vkAllocateCommandBuffers(this->m_device, &alloc_info, this->m_command_buffers.data()) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not allocate command buffers for rendering");
                }
            }
            void vulkan_context::create_sync_objects() {
                VkSemaphoreCreateInfo semaphore_info;
                util::zero(semaphore_info);
                semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                this->m_image_available_semaphores.resize(max_frames_in_flight);
                this->m_render_finished_semaphores.resize(max_frames_in_flight);
                for (size_t i = 0; i < max_frames_in_flight; i++) {
                    if ((vkCreateSemaphore(this->m_device, &semaphore_info, nullptr, &this->m_image_available_semaphores[i]) != VK_SUCCESS) ||
                        (vkCreateSemaphore(this->m_device, &semaphore_info, nullptr, &this->m_render_finished_semaphores[i]) != VK_SUCCESS)) {
                        throw std::runtime_error("[vulkan context] could not create sync objects");
                    }
                }
            }
            void vulkan_context::create_descriptor_pool() {
                std::vector<VkDescriptorPoolSize> pool_sizes = {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
                };
                VkDescriptorPoolCreateInfo create_info;
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                create_info.poolSizeCount = (uint32_t)pool_sizes.size();
                create_info.pPoolSizes = pool_sizes.data();
                create_info.maxSets = (uint32_t)(1000 * pool_sizes.size());
                create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
                if (vkCreateDescriptorPool(this->m_device, &create_info, nullptr, &this->m_descriptor_pool) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan context] could not create descriptor pool");
                }
            }
            void vulkan_context::create_depth_resources() {
                VkFormat format = find_depth_format(this->m_physical_device);
                create_image(this->m_swapchain_extent.width, this->m_swapchain_extent.height, format, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_device, this->m_physical_device,
                    this->m_depth_image, this->m_depth_image_memory);
                this->m_depth_image_view = create_image_view(this->m_depth_image, format, VK_IMAGE_ASPECT_DEPTH_BIT, this->m_device);
                transition_image_layout(this->m_depth_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, this);
            }
            void vulkan_context::cleanup_swapchain(bool* recreate_pipeline) {
                vkDestroyImageView(this->m_device, this->m_depth_image_view, nullptr);
                vkDestroyImage(this->m_device, this->m_depth_image, nullptr);
                vkFreeMemory(this->m_device, this->m_depth_image_memory, nullptr);
                vkFreeCommandBuffers(this->m_device, this->m_command_pool, (uint32_t)this->m_command_buffers.size(), this->m_command_buffers.data());
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
                if (recreate_pipeline) {
                    for (auto shader : vulkan_shader::get_active_shaders()) {
                        shader->destroy_descriptor_sets();
                    }
                }
                vkDestroyRenderPass(this->m_device, this->m_render_pass, nullptr);
                for (VkImageView image_view : this->m_swapchain_image_views) {
                    vkDestroyImageView(this->m_device, image_view, nullptr);
                }
                this->m_swapchain_image_views.clear();
                vkDestroySwapchainKHR(this->m_device, this->m_swap_chain, nullptr);
                vkDestroyDescriptorPool(this->m_device, this->m_descriptor_pool, nullptr);
            }
            void vulkan_context::recreate_swapchain(glm::ivec2 new_size) {
                vkDeviceWaitIdle(this->m_device);
                bool recreate_pipeline;
                this->cleanup_swapchain(&recreate_pipeline);
                this->create_swap_chain(new_size);
                this->create_image_views();
                this->create_render_pass();
                this->create_depth_resources();
                this->create_framebuffers();
                this->create_descriptor_pool();
                this->alloc_command_buffers();
                for (auto shader : vulkan_shader::get_active_shaders()) {
                    shader->create_descriptor_sets();
                }
                if (recreate_pipeline) {
                    auto pipeline = this->m_factory->m_current_pipeline.as<vulkan_pipeline>();
                    pipeline->create();
                }
                // imgui will crash; cant do anything about it now, i dont think
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
                std::vector<bool> requirements = {
                    (bool)features.geometryShader,
                    find_queue_families(device, this->m_window_surface).is_complete(),
                    extensions_supported,
                    swap_chain_adequate
                };
                for (bool satisfied : requirements) {
                    if (!satisfied) {
                        return 0;
                    }
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
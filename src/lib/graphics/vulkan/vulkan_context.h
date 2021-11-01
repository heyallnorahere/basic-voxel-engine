#pragma once
#include "graphics/context.h"
#include "vulkan_object_factory.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_context : public context {
            public:
                vulkan_context(ref<vulkan_object_factory> factory);
                virtual ~vulkan_context() override;
                virtual void clear(glm::vec4 clear_color) override;
                virtual void make_current() override;
                virtual void draw_indexed(size_t index_count) override;
                VkCommandBuffer begin_single_time_commands();
                void end_single_time_commands(VkCommandBuffer command_buffer);
                VkInstance get_instance() { return this->m_instance; }
                VkDebugUtilsMessengerEXT get_debug_messenger() { return this->m_debug_messenger; }
                VkPhysicalDevice get_physical_device() { return this->m_physical_device; }
                VkDevice get_device() { return this->m_device; }
                VkQueue get_graphics_queue() { return this->m_graphics_queue; }
                VkSurfaceKHR get_window_surface() { return this->m_window_surface; }
                VkSwapchainKHR get_swap_chain() { return this->m_swap_chain; }
                VkExtent2D get_swapchain_extent() { return this->m_swapchain_extent; }
                VkRenderPass get_render_pass() { return this->m_render_pass; }
                VkCommandPool get_command_pool() { return this->m_command_pool; }
                VkDescriptorPool get_descriptor_pool() { return this->m_descriptor_pool; }
                size_t get_swapchain_image_count() { return this->m_swapchain_images.size(); }
            private:
                virtual void swap_buffers() override;
                virtual void setup_glfw() override;
                virtual void setup_context() override;
                virtual void resize_viewport(int32_t x, int32_t y, int32_t width, int32_t height) override;
                virtual void init_imgui_backends() override;
                virtual void shutdown_imgui_backends() override;
                virtual void call_imgui_backend_newframe() override;
                virtual void render_imgui_draw_data(ImDrawData* data) override;
                void create_instance();
                void create_debug_messenger();
                void create_window_surface();
                void pick_physical_device();
                void create_logical_device();
                void create_swap_chain(glm::ivec2 size);
                void create_image_views();
                void create_render_pass();
                void create_framebuffers();
                void create_command_pool();
                void alloc_command_buffers();
                void create_sync_objects();
                void create_descriptor_pool();
                void create_depth_resources();
                void cleanup_swapchain(bool* recreate_pipeline);
                void recreate_swapchain(glm::ivec2 new_size);
                uint64_t rate_device(VkPhysicalDevice device);
                bool layers_supported();
                bool check_device_extension_support(VkPhysicalDevice device);
                std::vector<const char*> get_extensions();
                ref<vulkan_object_factory> m_factory;
                VkInstance m_instance;
                VkDebugUtilsMessengerEXT m_debug_messenger;
                VkPhysicalDevice m_physical_device;
                VkDevice m_device;
                VkQueue m_graphics_queue, m_present_queue;
                VkSurfaceKHR m_window_surface;
                VkSwapchainKHR m_swap_chain;
                uint32_t m_image_count, m_min_image_count;
                std::vector<VkImage> m_swapchain_images;
                VkFormat m_swapchain_image_format;
                VkExtent2D m_swapchain_extent;
                std::vector<VkImageView> m_swapchain_image_views;
                VkRenderPass m_render_pass;
                std::vector<VkFramebuffer> m_framebuffers;
                VkCommandPool m_command_pool;
                std::vector<ref<pipeline>> m_bound_pipelines;
                std::vector<VkCommandBuffer> m_command_buffers;
                std::vector<VkSemaphore> m_image_available_semaphores, m_render_finished_semaphores;
                VkDescriptorPool m_descriptor_pool;
                VkImage m_depth_image;
                VkImageView m_depth_image_view;
                VkDeviceMemory m_depth_image_memory;
                size_t m_current_frame;
                uint32_t m_current_image;
                std::vector<const char*> m_layers, m_extensions, m_device_extensions;
                bool m_validation_layers_enabled, m_resize_swapchain;
            };
        }
    }
}
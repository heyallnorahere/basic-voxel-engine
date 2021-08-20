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
                virtual void clear() override;
                virtual void make_current() override;
                virtual void draw_indexed(size_t index_count) override;
                VkInstance get_instance();
                VkDebugUtilsMessengerEXT get_debug_messenger();
                VkPhysicalDevice get_physical_device();
                VkDevice get_device();
                VkQueue get_graphics_queue();
                VkSurfaceKHR get_window_surface();
                VkSwapchainKHR get_swap_chain();
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
                void create_swap_chain(glm::ivec2 position, glm::ivec2 size);
                uint32_t rate_device(VkPhysicalDevice device);
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
                std::vector<const char*> m_layers, m_extensions, m_device_extensions;
                bool m_validation_layers_enabled;
            };
        }
    }
}
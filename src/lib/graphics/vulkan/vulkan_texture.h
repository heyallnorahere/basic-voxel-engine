#pragma once
#include "graphics/texture.h"
#include "vulkan_object_factory.h"
#include "vulkan_context.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice device, VkPhysicalDevice physical_device, VkImage& image, VkDeviceMemory& memory);
            void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, ref<vulkan_context> context);
            void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, ref<vulkan_context> context);
            VkImageView create_image_view(VkImage image, VkFormat format, VkDevice device);
            class vulkan_texture : public texture {
            public:
                static std::map<uint32_t, ref<vulkan_texture>> get_bound_textures();
                vulkan_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, ref<vulkan_object_factory> factory);
                virtual ~vulkan_texture() override;
                virtual void bind(uint32_t slot) override;
                virtual glm::ivec2 get_size() override;
                virtual int32_t get_channels() override;
                virtual ImTextureID get_texture_id() override;
                const VkDescriptorImageInfo& get_image_info() { return this->m_image_info; };
            private:
                void create(const std::vector<uint8_t>& data, ref<vulkan_context> context);
                void create_texture_image(const std::vector<uint8_t>& data, ref<vulkan_context> context, VkFormat& format);
                void create_sampler(ref<vulkan_context> context);
                ref<vulkan_object_factory> m_factory;
                VkDevice m_device;
                int32_t m_width, m_height, m_channels;
                VkImage m_image;
                VkImageView m_image_view;
                VkSampler m_sampler;
                VkDeviceMemory m_memory;
                VkDescriptorImageInfo m_image_info;
            };
        }
    }
}
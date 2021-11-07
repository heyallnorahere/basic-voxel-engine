#pragma once
#include "graphics/storage_buffer.h"
#include "vulkan_object_factory.h"
#include "vulkan_shader.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_storage_buffer : public storage_buffer {
            public:
                vulkan_storage_buffer(size_t size, uint32_t set, uint32_t binding, ref<vulkan_object_factory> factory);
                virtual ~vulkan_storage_buffer() override;
                virtual void set_data(const void* data, size_t size, size_t offset) override;
                virtual void get_data(void* data, size_t size, size_t offset) override;
                virtual void activate() override;
                void write_descriptor_set(ref<vulkan_shader> shader, uint32_t image_index);
            private:
                ref<vulkan_object_factory> m_factory;
                uint32_t m_set, m_binding;
                size_t m_size;
                VkBuffer m_buffer;
                VkDeviceMemory m_memory;
                VkDevice m_device;
                VkDescriptorBufferInfo m_descriptor_info;
            };
        }
    }
}
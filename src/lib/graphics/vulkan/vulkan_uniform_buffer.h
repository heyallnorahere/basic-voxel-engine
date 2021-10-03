#pragma once
#include "graphics/uniform_buffer.h"
#include "vulkan_object_factory.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_uniform_buffer : public uniform_buffer {
            public:
                static const std::list<ref<vulkan_uniform_buffer>>& get_active_uniform_buffers();
                vulkan_uniform_buffer(size_t size, uint32_t binding, ref<vulkan_object_factory> factory);
                virtual ~vulkan_uniform_buffer() override;
                virtual void set_data(const void* data, size_t size, size_t offset);
                virtual size_t get_size() override { return this->m_size; }
                virtual uint32_t get_binding() override { return this->m_binding; }
                const VkDescriptorBufferInfo& get_descriptor_info() { return this->m_descriptor_info; }
            private:
                ref<vulkan_object_factory> m_factory;
                VkBuffer m_buffer;
                VkDeviceMemory m_memory;
                VkDevice m_device;
                VkDescriptorBufferInfo m_descriptor_info;
                size_t m_size;
                uint32_t m_binding;
            };
        }
    }
}
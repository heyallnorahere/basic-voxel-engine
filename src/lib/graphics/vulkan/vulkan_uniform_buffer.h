#pragma once
#include "graphics/uniform_buffer.h"
#include "vulkan_object_factory.h"
#include "vulkan_shader.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_uniform_buffer : public uniform_buffer {
            public:
                vulkan_uniform_buffer(size_t size, uint32_t set, uint32_t binding, ref<vulkan_object_factory> factory);
                virtual ~vulkan_uniform_buffer() override;
                virtual void set_data(const void* data, size_t size, size_t offset) override;
                virtual size_t get_size() override { return this->m_size; }
                virtual uint32_t get_binding() override { return this->m_set * 16 + this->m_binding; }
                virtual void activate() override;
                void write_descriptor_set(ref<vulkan_shader> shader, uint32_t image_index);
            private:
                ref<vulkan_object_factory> m_factory;
                VkBuffer m_buffer;
                VkDeviceMemory m_memory;
                VkDevice m_device;
                VkDescriptorBufferInfo m_descriptor_info;
                size_t m_size;
                uint32_t m_binding, m_set;
            };
        }
    }
}
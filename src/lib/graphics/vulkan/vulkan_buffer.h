#pragma once
#include "graphics/buffer.h"
#include "vulkan_object_factory.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_buffer : public buffer {
            public:
                vulkan_buffer(const void* data, size_t size, VkBufferUsageFlags usage, ref<vulkan_object_factory> factory);
                virtual ~vulkan_buffer() override;
                virtual void bind() override;
                virtual void unbind() override;
                VkBuffer get_buffer() { return this->m_buffer; }
            private:
                ref<vulkan_object_factory> m_factory;
                VkBuffer m_buffer;
                VkDeviceMemory m_memory;
                VkDevice m_device;
                VkBufferUsageFlags m_usage;
            };
        }
    }
}
#include "bve_pch.h"
#include "vulkan_uniform_buffer.h"
#include "vulkan_buffer.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
#include "util.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            static std::list<vulkan_uniform_buffer*> active_uniform_buffers;
            std::vector<ref<vulkan_uniform_buffer>> vulkan_uniform_buffer::get_active_uniform_buffers() {
                std::vector<ref<vulkan_uniform_buffer>> refs;
                for (auto buf : active_uniform_buffers) {
                    refs.push_back(buf);
                }
                return refs;
            }
            vulkan_uniform_buffer::vulkan_uniform_buffer(size_t size, uint32_t binding, ref<vulkan_object_factory> factory) {
                this->m_size = size;
                this->m_binding = binding;
                this->m_factory = factory;
                auto context = this->m_factory->get_current_context();
                if (!context) {
                    throw std::runtime_error("[vulkan uniform buffer] attempted to create a uniform buffer without a context");
                }
                auto vk_context = context.as<vulkan_context>();
                this->m_device = vk_context->get_device();
                VkPhysicalDevice physical_device = vk_context->get_physical_device();
                create_buffer((VkDeviceSize)this->m_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->m_device, physical_device, this->m_buffer, this->m_memory);
                util::zero(this->m_descriptor_info);
                this->m_descriptor_info.buffer = this->m_buffer;
                this->m_descriptor_info.offset = 0;
                this->m_descriptor_info.range = (VkDeviceSize)this->m_size;
                active_uniform_buffers.push_back(this);
            }
            vulkan_uniform_buffer::~vulkan_uniform_buffer() {
                active_uniform_buffers.remove_if([this](vulkan_uniform_buffer* element) { return element == this; });
                vkDestroyBuffer(this->m_device, this->m_buffer, nullptr);
                vkFreeMemory(this->m_device, this->m_memory, nullptr);
            }
            void vulkan_uniform_buffer::set_data(const void* data, size_t size, size_t offset) {
                void* gpu_data;
                vkMapMemory(this->m_device, this->m_memory, 0, (VkDeviceSize)this->m_size, 0, &gpu_data);
                void* dest = (void*)((size_t)gpu_data + offset);
                memcpy(dest, data, size);
                vkUnmapMemory(this->m_device, this->m_memory);
            }
        }
    }
}
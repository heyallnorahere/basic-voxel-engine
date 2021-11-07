#include "bve_pch.h"
#include "vulkan_storage_buffer.h"
#include "vulkan_context.h"
#include "vulkan_buffer.h"
#include "vulkan_pipeline.h"
#include "util.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            vulkan_storage_buffer::vulkan_storage_buffer(size_t size, uint32_t set, uint32_t binding, ref<vulkan_object_factory> factory) {
                this->m_factory = factory;
                this->m_set = set;
                this->m_binding = binding;
                this->m_size = size;
                auto context = this->m_factory->get_current_context().as<vulkan_context>();
                this->m_device = context->get_device();
                VkPhysicalDevice physical_device = context->get_physical_device();
                create_buffer(this->m_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->m_device, physical_device, this->m_buffer, this->m_memory);
                util::zero(this->m_descriptor_info);
                this->m_descriptor_info.buffer = this->m_buffer;
                this->m_descriptor_info.range = this->m_size;
                this->m_descriptor_info.offset = 0;
            }
            vulkan_storage_buffer::~vulkan_storage_buffer() {
                vkDestroyBuffer(this->m_device, this->m_buffer, nullptr);
                vkFreeMemory(this->m_device, this->m_memory, nullptr);
            }
            void vulkan_storage_buffer::set_data(const void* data, size_t size, size_t offset = 0) {
                void* gpu_data;
                vkMapMemory(this->m_device, this->m_memory, offset, size, 0, &gpu_data);
                memcpy(gpu_data, data, size);
                vkUnmapMemory(this->m_device, this->m_memory);
            }
            void vulkan_storage_buffer::get_data(void* data, size_t size, size_t offset = 0) {
                void* gpu_data;
                vkMapMemory(this->m_device, this->m_memory, offset, size, 0, &gpu_data);
                memcpy(data, gpu_data, size);
                vkUnmapMemory(this->m_device, this->m_memory);
            }
            void vulkan_storage_buffer::activate() {
                auto context = this->m_factory->get_current_context().as<vulkan_context>();
                uint32_t image_index = context->get_current_image();
                auto pipeline = this->m_factory->get_current_pipeline().as<vulkan_pipeline>();
                auto shader = pipeline->get_shader();
                this->write_descriptor_set(shader, image_index);
            }
            void vulkan_storage_buffer::write_descriptor_set(ref<vulkan_shader> shader, uint32_t image_index) {
                const auto& descriptor_sets = shader->get_descriptor_sets();
                if (descriptor_sets.find(this->m_set) == descriptor_sets.end()) {
                    throw std::runtime_error("[vulkan storage buffer] tried to write to a nonexistent descriptor set");
                }
                VkDescriptorSet vk_descriptor_set = descriptor_sets.find(this->m_set)->second.sets[image_index];
                VkWriteDescriptorSet write;
                util::zero(write);
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = vk_descriptor_set;
                write.dstBinding = this->m_binding;
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &this->m_descriptor_info;
                vkUpdateDescriptorSets(this->m_device, 1, &write, 0, nullptr);
            }
        }
    }
}
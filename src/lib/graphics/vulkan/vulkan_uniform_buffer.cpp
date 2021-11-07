#include "bve_pch.h"
#include "vulkan_uniform_buffer.h"
#include "vulkan_buffer.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
#include "util.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
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
            }
            vulkan_uniform_buffer::~vulkan_uniform_buffer() {
                vkDestroyBuffer(this->m_device, this->m_buffer, nullptr);
                vkFreeMemory(this->m_device, this->m_memory, nullptr);
            }
            void vulkan_uniform_buffer::set_data(const void* data, size_t size, size_t offset) {
                void* gpu_data;
                vkMapMemory(this->m_device, this->m_memory, 0, this->m_size, 0, &gpu_data);
                void* dest = (void*)((size_t)gpu_data + offset);
                memcpy(dest, data, size);
                vkUnmapMemory(this->m_device, this->m_memory);
            }
            void vulkan_uniform_buffer::activate() {
                auto context = this->m_factory->get_current_context().as<vulkan_context>();
                uint32_t image_index = context->get_current_image();
                auto pipeline = this->m_factory->get_current_pipeline().as<vulkan_pipeline>();
                auto shader = pipeline->get_shader();
                this->write_descriptor_set(shader, image_index);
            }
            void vulkan_uniform_buffer::write_descriptor_set(ref<vulkan_shader> shader, uint32_t image_index) {
                const auto& reflection_data = shader->get_reflection_data();
                const auto& descriptor_sets = shader->get_descriptor_sets();
                if (reflection_data.uniform_buffers.find(this->m_binding) == reflection_data.uniform_buffers.end()) {
                    return;
                }
                const auto& buffer_info = reflection_data.uniform_buffers.find(this->m_binding)->second;
                uint32_t descriptor_set = buffer_info.descriptor_set;
                VkDescriptorSet vk_descriptor_set = descriptor_sets.find(descriptor_set)->second.sets[image_index];
                VkWriteDescriptorSet write;
                util::zero(write);
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.dstSet = vk_descriptor_set;
                write.dstBinding = this->m_binding;
                write.dstArrayElement = 0;
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.descriptorCount = 1;
                write.pBufferInfo = &this->m_descriptor_info;
                vkUpdateDescriptorSets(this->m_device, 1, &write, 0, nullptr);
            }
        }
    }
}
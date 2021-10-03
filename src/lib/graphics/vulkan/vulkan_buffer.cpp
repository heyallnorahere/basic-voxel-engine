#include "bve_pch.h"
#include "vulkan_buffer.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            static uint32_t find_memory_type(uint32_t filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical_device) {
                VkPhysicalDeviceMemoryProperties memory_properties;
                vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
                for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
                    if (filter & (1 << i)) {
                        return i;
                    }
                }
                throw std::runtime_error("[vulkan buffer] could not find suitable memory type");
                return 0;
            }
            static void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice device, VkPhysicalDevice physical_device, VkBuffer& buffer, VkDeviceMemory& memory) {
                VkBufferCreateInfo create_info;
                memset(&create_info, 0, sizeof(VkBufferCreateInfo));
                create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                create_info.size = (VkDeviceSize)size;
                create_info.usage = usage;
                create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                if (vkCreateBuffer(device, &create_info, nullptr, &buffer) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan buffer] could not create gpu buffer");
                }
                VkMemoryRequirements requirements;
                vkGetBufferMemoryRequirements(device, buffer, &requirements);
                VkMemoryAllocateInfo alloc_info;
                memset(&alloc_info, 0, sizeof(VkMemoryAllocateInfo));
                alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                alloc_info.allocationSize = requirements.size;
                alloc_info.memoryTypeIndex = find_memory_type(requirements.memoryTypeBits,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physical_device);
                if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan buffer] could not allocate memory on the gpu");
                }
                vkBindBufferMemory(device, buffer, memory, 0);
            }
            static void copy_buffer(VkBuffer src, VkBuffer dest, VkDeviceSize size, ref<vulkan_context> context) {
                VkDevice device = context->get_device();
                VkCommandPool command_pool = context->get_command_pool();
                VkCommandBufferAllocateInfo alloc_info;
                memset(&alloc_info, 0, sizeof(VkCommandBufferAllocateInfo));
                alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                alloc_info.commandPool = command_pool;
                alloc_info.commandBufferCount = 1;
                VkCommandBuffer command_buffer;
                if (vkAllocateCommandBuffers(device, &alloc_info, &command_buffer) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan buffer] could not allocate a command buffer for copying");
                }
                VkCommandBufferBeginInfo begin_info;
                memset(&begin_info, 0, sizeof(VkCommandBufferBeginInfo));
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan buffer] could not begin recording copy command");
                }
                VkBufferCopy copy_region;
                memset(&copy_region, 0, sizeof(VkBufferCopy));
                copy_region.srcOffset = 0;
                copy_region.dstOffset = 0;
                copy_region.size = size;
                vkCmdCopyBuffer(command_buffer, src, dest, 1, &copy_region);
                if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan buffer] could not end recording of copy command");
                }
                VkSubmitInfo submit_info;
                memset(&submit_info, 0, sizeof(VkSubmitInfo));
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &command_buffer;
                VkQueue queue = context->get_graphics_queue();
                if (vkQueueSubmit(queue, 1, &submit_info, nullptr) != VK_SUCCESS || vkQueueWaitIdle(queue) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan buffer] could not copy buffers");
                }
                vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
            }
            vulkan_buffer::vulkan_buffer(const void* data, size_t size, VkBufferUsageFlags usage, ref<vulkan_object_factory> factory) {
                this->m_factory = factory;
                this->m_usage = usage;
                auto context = this->m_factory->get_current_context().as<vulkan_context>();
                this->m_device = context->get_device();
                VkPhysicalDevice physical_device = context->get_physical_device();
                VkBuffer staging_buffer;
                VkDeviceMemory staging_memory;
                VkDeviceSize buffer_size = (VkDeviceSize)size;
                create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    this->m_device, physical_device, staging_buffer, staging_memory);
                void* gpu_data;
                vkMapMemory(this->m_device, staging_memory, 0, buffer_size, 0, &gpu_data);
                memcpy(gpu_data, data, size);
                vkUnmapMemory(this->m_device, staging_memory);
                create_buffer(buffer_size, this->m_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->m_device, physical_device, this->m_buffer, this->m_memory);
                copy_buffer(staging_buffer, this->m_buffer, buffer_size, context);
                vkDestroyBuffer(this->m_device, staging_buffer, nullptr);
                vkFreeMemory(this->m_device, staging_memory, nullptr);
            }
            vulkan_buffer::~vulkan_buffer() {
                vkDestroyBuffer(this->m_device, this->m_buffer, nullptr);
                vkFreeMemory(this->m_device, this->m_memory, nullptr);
            }
            void vulkan_buffer::bind() {
                auto pipeline = this->m_factory->get_current_pipeline();
                if (pipeline) {
                    auto vk_pipeline = pipeline.as<vulkan_pipeline>();
                    vk_pipeline->bind_buffer(this->m_usage, this);
                } else {
                    spdlog::warn("[vulkan buffer] attempted to bind a buffer to nullptr");
                }
            }
            void vulkan_buffer::unbind() {
                auto pipeline = this->m_factory->get_current_pipeline();
                if (pipeline) {
                    auto vk_pipeline = pipeline.as<vulkan_pipeline>();
                    vk_pipeline->unbind_buffer(this->m_usage);
                }
            }
        }
    }
}
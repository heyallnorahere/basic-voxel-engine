#include "bve_pch.h"
#include "vulkan_compute_pipeline.h"
#include "vulkan_context.h"
#include "vulkan_uniform_buffer.h"
#include "util.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            vulkan_compute_pipeline::vulkan_compute_pipeline(ref<vulkan_shader> shader_, ref<vulkan_object_factory> factory) {
                this->m_factory = factory;
                this->m_shader = shader_;
                auto context = this->m_factory->get_current_context().as<vulkan_context>();
                this->m_device = context->get_device();
                this->m_queue = context->get_compute_queue();
                auto indices = find_queue_families(context->get_physical_device(), context->get_window_surface());
                std::vector<VkDescriptorSetLayout> set_layouts;
                for (const auto& [id, set] : this->m_shader->get_descriptor_sets()) {
                    set_layouts.push_back(set.layout);
                }
                VkPipelineLayoutCreateInfo layout_create_info;
                util::zero(layout_create_info);
                layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                layout_create_info.setLayoutCount = set_layouts.size();
                layout_create_info.pSetLayouts = set_layouts.data();
                if (vkCreatePipelineLayout(this->m_device, &layout_create_info, nullptr, &this->m_layout) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan compute pipeline] could not create pipeline layout");
                }
                VkComputePipelineCreateInfo pipeline_create_info;
                util::zero(pipeline_create_info);
                pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                pipeline_create_info.layout = this->m_layout;
                for (const auto& stage : this->m_shader->get_create_info()) {
                    if (stage.stage & VK_SHADER_STAGE_COMPUTE_BIT) {
                        pipeline_create_info.stage = stage;
                    }
                }
                if (!pipeline_create_info.stage.module) {
                    throw std::runtime_error("[vulkan compute pipeline] the given shader does not have a compute stage");
                }
                if (vkCreateComputePipelines(this->m_device, nullptr, 1, &pipeline_create_info, nullptr, &this->m_pipeline) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan compute pipeline] could not create compute pipeline");
                }
                VkCommandPoolCreateInfo cmd_pool_create_info;
                util::zero(cmd_pool_create_info);
                cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                cmd_pool_create_info.queueFamilyIndex = *indices.compute_family;
                cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                if (vkCreateCommandPool(this->m_device, &cmd_pool_create_info, nullptr, &this->m_command_pool) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan compute pipeline] could not create command pool");
                }
                VkCommandBufferAllocateInfo cmd_buffer_alloc_info;
                util::zero(cmd_buffer_alloc_info);
                cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                cmd_buffer_alloc_info.commandPool = this->m_command_pool;
                cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                cmd_buffer_alloc_info.commandBufferCount = 1;
                if (vkAllocateCommandBuffers(this->m_device, &cmd_buffer_alloc_info, &this->m_command_buffer) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan compute pipeline] could not allocate command buffer");
                }
            }
            vulkan_compute_pipeline::~vulkan_compute_pipeline() {
                vkFreeCommandBuffers(this->m_device, this->m_command_pool, 1, &this->m_command_buffer);
                vkDestroyCommandPool(this->m_device, this->m_command_pool, nullptr);
                vkDestroyPipeline(this->m_device, this->m_pipeline, nullptr);
                vkDestroyPipelineLayout(this->m_device, this->m_layout, nullptr);
            }
            void vulkan_compute_pipeline::bind_uniform_buffer(ref<uniform_buffer> buffer) {
                // i probably shouldnt let it create multiple sets per shader set
                // but its not exactly an urgent problem
                auto ubo = buffer.as<vulkan_uniform_buffer>();
                ubo->write_descriptor_set(this->m_shader, 0);
            }
            void vulkan_compute_pipeline::dispatch(glm::uvec3 group_count) {
                vkQueueWaitIdle(this->m_queue);
                VkCommandBufferBeginInfo begin_info;
                util::zero(begin_info);
                begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                if (vkBeginCommandBuffer(this->m_command_buffer, &begin_info) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan compute pipeline] could not begin command buffer");
                }
                vkCmdBindPipeline(this->m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->m_pipeline);
                std::vector<VkDescriptorSet> sets;
                for (const auto& [id, set] : this->m_shader->get_descriptor_sets()) {
                    sets.push_back(set.sets[0]);
                }
                vkCmdBindDescriptorSets(this->m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->m_layout, 0, sets.size(), sets.data(), 0, nullptr);
                vkCmdDispatch(this->m_command_buffer, group_count.x, group_count.y, group_count.z);
                if (vkEndCommandBuffer(this->m_command_buffer) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan compute pipeline] could not end command buffer");
                }
                VkFenceCreateInfo fence_create_info;
                util::zero(fence_create_info);
                fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                VkFence fence;
                if (vkCreateFence(this->m_device, &fence_create_info, nullptr, &fence) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan compute pipeline] could not create fence");
                }
                VkSubmitInfo submit_info;
                util::zero(submit_info);
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &this->m_command_buffer;
                if (vkQueueSubmit(this->m_queue, 1, &submit_info, fence) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan compute pipeline] could not submit command buffer");
                }
                vkWaitForFences(this->m_device, 1, &fence, true, UINT64_MAX);
                vkDestroyFence(this->m_device, fence, nullptr);
                vkResetCommandBuffer(this->m_command_buffer, 0);
            }
        }
    }
}
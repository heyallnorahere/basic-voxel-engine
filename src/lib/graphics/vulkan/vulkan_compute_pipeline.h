#pragma once
#include "vulkan_object_factory.h"
#include "vulkan_shader.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_compute_pipeline : public compute_pipeline {
            public:
                vulkan_compute_pipeline(ref<vulkan_shader> shader_, ref<vulkan_object_factory> factory);
                virtual ~vulkan_compute_pipeline() override;
                virtual void bind_uniform_buffer(ref<uniform_buffer> buffer) override;
                virtual void dispatch(glm::uvec3 group_count) override;
            private:
                ref<vulkan_object_factory> m_factory;
                ref<vulkan_shader> m_shader;
                VkDevice m_device;
                VkQueue m_queue;
                VkPipeline m_pipeline;
                VkPipelineLayout m_layout;
                VkCommandPool m_command_pool;
                VkCommandBuffer m_command_buffer;
            };
        }
    }
}
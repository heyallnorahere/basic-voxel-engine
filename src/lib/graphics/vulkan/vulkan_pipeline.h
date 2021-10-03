#pragma once
#include "vulkan_object_factory.h"
#include "vulkan_shader.h"
#include "vulkan_context.h"
#include "vulkan_buffer.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            class vulkan_pipeline : public pipeline {
            public:
                vulkan_pipeline(ref<vulkan_object_factory> factory);
                virtual ~vulkan_pipeline() override;
                virtual void set_vertex_attributes(const std::vector<vertex_attribute>& attributes) override;
                virtual void bind() override;
                virtual void unbind() override;
                VkPipeline get_pipeline() { return this->m_pipeline; }
                void set_shader(ref<vulkan_shader> shader);
                void bind_buffer(VkBufferUsageFlags type, ref<vulkan_buffer> buffer);
                void unbind_buffer(VkBufferUsageFlags type);
                std::map<VkBufferUsageFlags, ref<vulkan_buffer>> get_bound_buffers();
                void create();
                void destroy();
                bool valid() { return this->m_pipeline != nullptr && this->m_layout != nullptr; }
            private:
                ref<vulkan_object_factory> m_factory;
                ref<vulkan_context> m_context;
                ref<vulkan_shader> m_shader;
                std::map<VkBufferUsageFlags, ref<vulkan_buffer>> m_buffers;
                std::vector<vertex_attribute> m_vertex_attributes;
                VkPipeline m_pipeline;
                VkPipelineLayout m_layout;
            };
        }
    }
}
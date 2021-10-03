#pragma once
#include "vulkan_object_factory.h"
#include "vulkan_shader.h"
#include "vulkan_context.h"
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
                void set_shader(ref<vulkan_shader> shader);
                void create();
            private:
                void destroy();
                ref<vulkan_object_factory> m_factory;
                ref<vulkan_context> m_context;
                ref<vulkan_shader> m_shader;
                VkPipeline m_pipeline;
                VkPipelineLayout m_layout;
            };
        }
    }
}
#include "bve_pch.h"
#include "vulkan_pipeline.h"
#include "vulkan_context.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            vulkan_pipeline::vulkan_pipeline(ref<vulkan_object_factory> factory) {
                this->m_factory = factory;
                this->m_pipeline = nullptr;
            }
            vulkan_pipeline::~vulkan_pipeline() {
                if (this->m_pipeline) {
                    this->destroy();
                }
            }
            void vulkan_pipeline::set_vertex_attributes(const std::vector<vertex_attribute>& attributes) {
                // todo: implement
            }
            void vulkan_pipeline::bind() {
                this->m_factory->m_current_pipeline = this;
                this->m_context = this->m_factory->m_current_context;
            }
            void vulkan_pipeline::unbind() {
                if (this->m_factory->m_current_pipeline == this) {
                    this->m_factory->m_current_pipeline.reset();
                }
            }
            void vulkan_pipeline::set_shader(ref<vulkan_shader> shader) {
                this->m_shader = shader;
                this->build();
            }
            void vulkan_pipeline::build() {
                if (this->m_pipeline) {
                    this->destroy();
                }
                if (!this->m_context) {
                    this->m_context = this->m_factory->m_current_context.as<vulkan_context>();
                }
                VkExtent2D swapchain_extent = this->m_context->get_swapchain_extent();
                VkPipelineVertexInputStateCreateInfo vertex_input_info;
                memset(&vertex_input_info, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
                vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertex_input_info.vertexBindingDescriptionCount = 0;
                vertex_input_info.pVertexBindingDescriptions = nullptr;
                vertex_input_info.vertexAttributeDescriptionCount = 0;
                vertex_input_info.pVertexAttributeDescriptions = nullptr;
                VkViewport viewport;
                memset(&viewport, 0, sizeof(VkViewport));
                viewport.x = viewport.y = 0.f;
                viewport.width = (float)swapchain_extent.width;
                viewport.height = (float)swapchain_extent.height;
                viewport.minDepth = 0.f;
                viewport.maxDepth = 1.f;
                VkRect2D scissor;
                memset(&scissor, 0, sizeof(VkRect2D));
                scissor.extent = swapchain_extent;
                VkPipelineViewportStateCreateInfo viewport_state;
                memset(&viewport_state, 0, sizeof(VkPipelineViewportStateCreateInfo));
                viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewport_state.viewportCount = 1;
                viewport_state.pViewports = &viewport;
                viewport_state.scissorCount = 1;
                viewport_state.pScissors = &scissor;
                VkPipelineRasterizationStateCreateInfo rasterizer;
                memset(&rasterizer, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
                rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable = false;
                rasterizer.rasterizerDiscardEnable = false;
                rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
                rasterizer.lineWidth = 1.f;
                rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
                rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                rasterizer.depthBiasEnable = false;
                VkPipelineMultisampleStateCreateInfo multisampling;
                memset(&multisampling, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
                multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampling.sampleShadingEnable = false;
                multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                multisampling.minSampleShading = 1.f;
                multisampling.pSampleMask = nullptr;
                multisampling.alphaToCoverageEnable = false;
                multisampling.alphaToOneEnable = false;
                VkPipelineColorBlendAttachmentState color_blend_attachment;
                memset(&color_blend_attachment, 0, sizeof(VkPipelineColorBlendAttachmentState));
                color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                color_blend_attachment.blendEnable = true;
                color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
                color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
                VkPipelineColorBlendStateCreateInfo color_blending;
                memset(&color_blending, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
                color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                color_blending.logicOpEnable = false;
                color_blending.logicOp = VK_LOGIC_OP_COPY;
                color_blending.attachmentCount = 1;
                color_blending.pAttachments = &color_blend_attachment;
                for (size_t i = 0; i < 4; i++) {
                    color_blending.blendConstants[i] = 0.f;
                }
                std::vector<VkDynamicState> dynamic_states = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_LINE_WIDTH
                };
                VkPipelineDynamicStateCreateInfo dynamic_state;
                memset(&dynamic_state, 0, sizeof(VkPipelineDynamicStateCreateInfo));
                dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamic_state.dynamicStateCount = (uint32_t)dynamic_states.size();
                dynamic_state.pDynamicStates = dynamic_states.data();
                VkPipelineLayoutCreateInfo pipeline_layout_info;
                memset(&pipeline_layout_info, 0, sizeof(VkPipelineLayoutCreateInfo));
                pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pipeline_layout_info.setLayoutCount = 0;
                pipeline_layout_info.pSetLayouts = nullptr;
                pipeline_layout_info.pushConstantRangeCount = 0;
                pipeline_layout_info.pPushConstantRanges = nullptr;
                if (vkCreatePipelineLayout(this->m_context->get_device(), &pipeline_layout_info, nullptr, &this->m_layout) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan pipeline] could not create pipeline layout");
                }
                // todo: create the damn pipeline
            }
            void vulkan_pipeline::destroy() {
                VkDevice device = this->m_context->get_device();
                vkDestroyPipelineLayout(device, this->m_layout, nullptr);
                vkDestroyPipeline(device, this->m_pipeline, nullptr);
            }
        }
    }
}
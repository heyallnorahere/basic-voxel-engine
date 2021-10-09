#include "bve_pch.h"
#include "vulkan_pipeline.h"
#include "vulkan_context.h"
#include "util.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            vulkan_pipeline::vulkan_pipeline(ref<vulkan_object_factory> factory) {
                this->m_factory = factory;
                this->m_pipeline = nullptr;
                this->m_layout = nullptr;
            }
            vulkan_pipeline::~vulkan_pipeline() {
                if (this->m_pipeline) {
                    this->destroy();
                }
            }
            void vulkan_pipeline::set_vertex_attributes(const std::vector<vertex_attribute>& attributes) {
                this->m_vertex_attributes = attributes;
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
            void vulkan_pipeline::bind_buffer(VkBufferUsageFlags type, ref<vulkan_buffer> buffer) {
                if (buffer) {
                    this->m_buffers.insert({ type, buffer });
                }
            }
            void vulkan_pipeline::unbind_buffer(VkBufferUsageFlags type) {
                if (this->m_buffers.find(type) != this->m_buffers.end()) {
                    this->m_buffers.erase(type);
                }
            }
            void vulkan_pipeline::create() {
                if (this->m_factory->m_current_pipeline != this) {
                    this->bind();
                }
                VkExtent2D swapchain_extent = this->m_context->get_swapchain_extent();
                VkDevice device = this->m_context->get_device();
                VkPipelineVertexInputStateCreateInfo vertex_input_info;
                util::zero(vertex_input_info);
                vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                std::vector<VkVertexInputAttributeDescription> attributes;
                VkVertexInputBindingDescription binding_description;
                if (this->m_vertex_attributes.empty()) { 
                    vertex_input_info.vertexBindingDescriptionCount = 0;
                    vertex_input_info.pVertexBindingDescriptions = nullptr;
                    vertex_input_info.vertexAttributeDescriptionCount = 0;
                    vertex_input_info.pVertexAttributeDescriptions = nullptr;
                } else {
                    util::zero(binding_description);
                    binding_description.binding = 0;
                    binding_description.stride = (uint32_t)this->m_vertex_attributes[0].stride;
                    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                    for (uint32_t i = 0; i < this->m_vertex_attributes.size(); i++) {
                        const auto& attr = this->m_vertex_attributes[i];
                        VkVertexInputAttributeDescription descriptor;
                        util::zero(descriptor);
                        descriptor.binding = 0;
                        descriptor.location = i;
                        switch (attr.type) {
                        case vertex_attribute_type::INT:
                            descriptor.format = VK_FORMAT_R32_SINT;
                            break;
                        case vertex_attribute_type::FLOAT:
                            descriptor.format = VK_FORMAT_R32_SFLOAT;
                            break;
                        case vertex_attribute_type::VEC2:
                            descriptor.format = VK_FORMAT_R32G32_SFLOAT;
                            break;
                        case vertex_attribute_type::VEC3:
                            descriptor.format = VK_FORMAT_R32G32B32_SFLOAT;
                            break;
                        case vertex_attribute_type::VEC4:
                            descriptor.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                            break;
                        case vertex_attribute_type::IVEC2:
                            descriptor.format = VK_FORMAT_R32G32_SINT;
                            break;
                        case vertex_attribute_type::IVEC3:
                            descriptor.format = VK_FORMAT_R32G32B32_SINT;
                            break;
                        case vertex_attribute_type::IVEC4:
                            descriptor.format = VK_FORMAT_R32G32B32A32_SINT;
                            break;
                        default:
                            throw std::runtime_error("[vulkan pipeline] unsupported vertex attribute type");
                        }
                        descriptor.offset = (uint32_t)attr.offset;
                        attributes.push_back(descriptor);
                    }
                    vertex_input_info.vertexBindingDescriptionCount = 1;
                    vertex_input_info.pVertexBindingDescriptions = &binding_description;
                    vertex_input_info.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
                    vertex_input_info.pVertexAttributeDescriptions = attributes.data();
                }
                VkPipelineInputAssemblyStateCreateInfo input_assembly;
                util::zero(input_assembly);
                input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                input_assembly.primitiveRestartEnable = false;
                VkViewport viewport;
                util::zero(viewport);
                viewport.x = viewport.y = 0.f;
                viewport.width = (float)swapchain_extent.width;
                viewport.height = (float)swapchain_extent.height;
                viewport.minDepth = 0.f;
                viewport.maxDepth = 1.f;
                VkRect2D scissor;
                util::zero(scissor);
                scissor.extent = swapchain_extent;
                VkPipelineViewportStateCreateInfo viewport_state;
                util::zero(viewport_state);
                viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewport_state.viewportCount = 1;
                viewport_state.pViewports = &viewport;
                viewport_state.scissorCount = 1;
                viewport_state.pScissors = &scissor;
                VkPipelineRasterizationStateCreateInfo rasterizer;
                util::zero(rasterizer);
                rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable = false;
                rasterizer.rasterizerDiscardEnable = false;
                rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
                rasterizer.lineWidth = 1.f;
                rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
                rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                rasterizer.depthBiasEnable = false;
                VkPipelineMultisampleStateCreateInfo multisampling;
                util::zero(multisampling);
                multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampling.sampleShadingEnable = false;
                multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                multisampling.minSampleShading = 1.f;
                multisampling.pSampleMask = nullptr;
                multisampling.alphaToCoverageEnable = false;
                multisampling.alphaToOneEnable = false;
                VkPipelineDepthStencilStateCreateInfo depth_stencil;
                util::zero(depth_stencil);
                depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                depth_stencil.depthTestEnable = true;
                depth_stencil.depthWriteEnable = true;
                depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
                depth_stencil.depthBoundsTestEnable = false;
                depth_stencil.minDepthBounds = 0.f;
                depth_stencil.maxDepthBounds = 1.f;
                depth_stencil.stencilTestEnable = false;
                VkPipelineColorBlendAttachmentState color_blend_attachment;
                util::zero(color_blend_attachment);
                color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                color_blend_attachment.blendEnable = true;
                color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
                color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
                VkPipelineColorBlendStateCreateInfo color_blending;
                util::zero(color_blending);
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
                util::zero(dynamic_state);
                dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamic_state.dynamicStateCount = (uint32_t)dynamic_states.size();
                dynamic_state.pDynamicStates = dynamic_states.data();
                VkPipelineLayoutCreateInfo pipeline_layout_info;
                util::zero(pipeline_layout_info);
                pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                std::vector<VkDescriptorSetLayout> layouts;
                if (this->m_shader) {
                    const auto& sets = this->m_shader->get_descriptor_sets();
                    for (const auto& set : sets) {
                        layouts.push_back(set.layout);
                    }
                    pipeline_layout_info.setLayoutCount = (uint32_t)layouts.size();
                    pipeline_layout_info.pSetLayouts = layouts.data();
                }
                pipeline_layout_info.pushConstantRangeCount = 0;
                pipeline_layout_info.pPushConstantRanges = nullptr;
                if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &this->m_layout) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan pipeline] could not create pipeline layout");
                }
                VkGraphicsPipelineCreateInfo create_info;
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                if (this->m_shader) {
                    const auto& shader_stage_create_info = this->m_shader->get_create_info();
                    create_info.stageCount = (uint32_t)shader_stage_create_info.size();
                    create_info.pStages = shader_stage_create_info.data();
                } else {
                    throw std::runtime_error("[vulkan pipeline] cannot create a pipeline without a shader");
                }
                create_info.pVertexInputState = &vertex_input_info;
                create_info.pInputAssemblyState = &input_assembly;
                create_info.pViewportState = &viewport_state;
                create_info.pRasterizationState = &rasterizer;
                create_info.pMultisampleState = &multisampling;
                create_info.pDepthStencilState = &depth_stencil;
                create_info.pColorBlendState = &color_blending;
                //create_info.pDynamicState = &dynamic_state;
                create_info.layout = this->m_layout;
                create_info.renderPass = this->m_context->get_render_pass();
                create_info.subpass = 0;
                create_info.basePipelineHandle = nullptr;
                create_info.basePipelineIndex = -1;
                if (vkCreateGraphicsPipelines(device, nullptr, 1, &create_info, nullptr, &this->m_pipeline) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan pipeline] could not create pipeline");
                }
            }
            void vulkan_pipeline::destroy() {
                VkDevice device = this->m_context->get_device();
                vkDestroyPipeline(device, this->m_pipeline, nullptr);
                vkDestroyPipelineLayout(device, this->m_layout, nullptr);
                this->m_pipeline = nullptr;
                this->m_layout = nullptr;
            }
        }
    }
}
#include "bve_pch.h"
#include "vulkan_shader.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
#include "vulkan_uniform_buffer.h"
#include "vulkan_texture.h"
#include "../../shader_compiler.h"
#include "util.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            static VkShaderStageFlagBits get_stage_flags(shader_type stage) {
                switch (stage) {
                case shader_type::VERTEX:
                    return VK_SHADER_STAGE_VERTEX_BIT;
                    break;
                case shader_type::FRAGMENT:
                    return VK_SHADER_STAGE_FRAGMENT_BIT;
                    break;
                case shader_type::GEOMETRY:
                    return VK_SHADER_STAGE_GEOMETRY_BIT;
                    break;
                default:
                    throw std::runtime_error("[vulkan shader] invalid shader type");
                    return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
                }
            }
            static std::list<vulkan_shader*> active_shaders;
            std::vector<ref<vulkan_shader>> vulkan_shader::get_active_shaders() {
                std::vector<ref<vulkan_shader>> refs;
                for (const auto& shader : active_shaders) {
                    refs.push_back(shader);
                }
                return refs;
            }
            vulkan_shader::vulkan_shader(ref<vulkan_object_factory> factory, const std::vector<fs::path>& sources) {
                this->m_device = nullptr;
                this->m_factory = factory;
                this->m_sources = sources;
                this->compile();
                active_shaders.push_back(this);
            }
            vulkan_shader::~vulkan_shader() {
                active_shaders.remove_if([this](vulkan_shader* element) { return element == this; });
                this->cleanup();
            }
            void vulkan_shader::reload() {
                this->cleanup();
                this->compile();
            }
            void vulkan_shader::bind() {
                auto pipeline = this->m_factory->get_current_pipeline();
                if (pipeline) {
                    auto vk_pipeline = pipeline.as<vulkan_pipeline>();
                    vk_pipeline->set_shader(this);
                    vk_pipeline->create();
                } else {
                    spdlog::warn("[vulkan shader] attempted to bind to a null pipeline");
                }
            }
            void vulkan_shader::unbind() { }
            void vulkan_shader::set_int(const std::string& name, int32_t value) { }
            void vulkan_shader::set_float(const std::string& name, float value) { }
            void vulkan_shader::set_ivec2(const std::string& name, const glm::ivec2& value) { }
            void vulkan_shader::set_ivec3(const std::string& name, const glm::ivec3& value) { }
            void vulkan_shader::set_ivec4(const std::string& name, const glm::ivec4& value) { }
            void vulkan_shader::set_vec2(const std::string& name, const glm::vec2& value) { }
            void vulkan_shader::set_vec3(const std::string& name, const glm::vec3& value) { }
            void vulkan_shader::set_vec4(const std::string& name, const glm::vec4& value) { }
            void vulkan_shader::set_mat4(const std::string& name, const glm::mat4& value) { }
            int32_t vulkan_shader::get_int(const std::string& name) { return 0; }
            float vulkan_shader::get_float(const std::string& name) { return 0.f; }
            glm::ivec2 vulkan_shader::get_ivec2(const std::string& name) { return glm::ivec2(0); }
            glm::ivec3 vulkan_shader::get_ivec3(const std::string& name) { return glm::ivec3(0); }
            glm::ivec4 vulkan_shader::get_ivec4(const std::string& name) { return glm::ivec4(0); }
            glm::vec2 vulkan_shader::get_vec2(const std::string& name) { return glm::vec2(0.f); }
            glm::vec3 vulkan_shader::get_vec3(const std::string& name) { return glm::vec3(0.f); }
            glm::vec4 vulkan_shader::get_vec4(const std::string& name) { return glm::vec4(0.f); }
            glm::mat4 vulkan_shader::get_mat4(const std::string& name) { return glm::mat4(1.f); }
            void vulkan_shader::compile() {
                ref<vulkan_context> context_ = this->m_factory->get_current_context();
                this->m_device = context_->get_device();
                auto parser = shader_parser(shader_parser::get_language(this->m_sources), shader_language::GLSL);
                for (const auto& source : this->m_sources) {
                    parser.parse(source);
                }
                std::vector<std::pair<shader_type, VkShaderModule>> shader_modules;
                for (shader_type type : parser.get_parsed_shader_types()) {
                    auto optional_path = parser.get_shader_path(type);
                    std::string path, shader_name;
                    if (optional_path) {
                        path = optional_path->string();
                    } else {
                        path = "unknown";
                    }
                    switch (type) {
                    case shader_type::VERTEX:
                        shader_name = "vertex";
                        break;
                    case shader_type::FRAGMENT:
                        shader_name = "fragment";
                        break;
                    case shader_type::GEOMETRY:
                        shader_name = "geometry";
                        break;
                    default:
                        shader_name = "unidentified";
                        break;
                    }
                    spdlog::info("[vulkan shader] compiling {0} shader (path: {1})", shader_name, path);
                    shader_modules.push_back({ type, this->compile_shader(type, parser) });
                }
                for (const auto& module : shader_modules) {
                    VkPipelineShaderStageCreateInfo create_info;
                    util::zero(create_info);
                    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    create_info.stage = get_stage_flags(module.first);
                    create_info.pName = "main";
                    create_info.module = module.second;
                    this->m_pipeline_create_info.push_back(create_info);
                }
                this->create_descriptor_sets();
            }
            VkShaderModule vulkan_shader::compile_shader(shader_type type, const shader_parser& parser) {
                std::string source = parser.get_shader(type);
                shader_compiler compiler;
                auto spirv = compiler.compile(source, shader_language::GLSL, type);
                this->reflect(type, spirv);
                VkShaderModuleCreateInfo create_info;
                util::zero(create_info);
                create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                create_info.codeSize = spirv.size() * sizeof(uint32_t);
                create_info.pCode = spirv.data();
                VkShaderModule shader_module;
                if (vkCreateShaderModule(this->m_device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan shader] could not create shader module");
                }
                return shader_module;
            }
            void vulkan_shader::cleanup() {
                this->destroy_descriptor_sets();
                for (const auto& create_info : this->m_pipeline_create_info) {
                    vkDestroyShaderModule(this->m_device, create_info.module, nullptr);
                }
                this->m_pipeline_create_info.clear();
            }
            struct layout_binding_t {
                VkDescriptorType descriptor_type;
                VkShaderStageFlagBits stage;
                uint32_t descriptor_count;
            };
            void vulkan_shader::create_descriptor_sets() {
                auto context = this->m_factory->get_current_context().as<vulkan_context>();
                size_t image_count = context->get_swapchain_image_count();
                this->m_descriptor_pool = context->get_descriptor_pool();
                const auto& data = this->get_reflection_data();
                uint32_t descriptor_set_count = data.get_descriptor_set_count();
                if (descriptor_set_count == 0) {
                    return;
                }
                std::map<uint32_t, std::map<uint32_t, layout_binding_t>> layout_bindings;
                for (const auto& [binding, buffer_info] : data.uniform_buffers) {
                    layout_binding_t layout_binding;
                    layout_binding.descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    layout_binding.stage = get_stage_flags(buffer_info.stage);
                    layout_binding.descriptor_count = 1;
                    layout_bindings[buffer_info.descriptor_set][binding] = layout_binding;
                }
                for (const auto& [binding, resource_info] : data.sampled_images) {
                    layout_binding_t layout_binding;
                    layout_binding.descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    layout_binding.stage = get_stage_flags(resource_info.stage);
                    layout_binding.descriptor_count = resource_info.type->array_size;
                    layout_bindings[resource_info.descriptor_set][binding] = layout_binding;
                }
                std::vector<VkDescriptorSetLayout> layouts;
                for (uint32_t i = 0; i < descriptor_set_count; i++) {
                    std::vector<VkDescriptorSetLayoutBinding> bindings;
                    for (const auto& [binding, data] : layout_bindings[i]) {
                        VkDescriptorSetLayoutBinding layout_binding;
                        util::zero(layout_binding);
                        layout_binding.binding = binding;
                        layout_binding.descriptorType = data.descriptor_type;
                        layout_binding.descriptorCount = data.descriptor_count;
                        layout_binding.stageFlags = data.stage;
                        layout_binding.pImmutableSamplers = nullptr;
                        bindings.push_back(layout_binding);
                    }
                    VkDescriptorSetLayoutCreateInfo create_info;
                    util::zero(create_info);
                    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    create_info.bindingCount = (uint32_t)bindings.size();
                    create_info.pBindings = bindings.data();
                    VkDescriptorSetLayout layout;
                    if (vkCreateDescriptorSetLayout(this->m_device, &create_info, nullptr, &layout) != VK_SUCCESS) {
                        throw std::runtime_error("[vulkan shader] could not create descriptor set layout for binding " + std::to_string(i));
                    }
                    this->m_descriptor_sets.push_back({ layout });
                    for (size_t j = 0; j < image_count; j++) {
                        layouts.push_back(layout);
                    }
                }
                VkDescriptorSetAllocateInfo alloc_info;
                util::zero(alloc_info);
                alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                alloc_info.descriptorPool = this->m_descriptor_pool;
                alloc_info.descriptorSetCount = (uint32_t)layouts.size();
                alloc_info.pSetLayouts = layouts.data();
                std::vector<VkDescriptorSet> sets(layouts.size());
                if (vkAllocateDescriptorSets(this->m_device, &alloc_info, sets.data()) != VK_SUCCESS) {
                    throw std::runtime_error("[vulkan shader] could not allocate descriptor sets");
                }
                for (size_t i = 0; i < descriptor_set_count; i++) {
                    for (size_t j = 0; j < image_count; j++) {
                        size_t set_index = (i * image_count) + j;
                        this->m_descriptor_sets[i].sets.push_back(sets[set_index]);
                    }
                }
            }
            void vulkan_shader::destroy_descriptor_sets() {
                std::vector<VkDescriptorSet> sets;
                for (const auto& desc : this->m_descriptor_sets) {
                    vkDestroyDescriptorSetLayout(this->m_device, desc.layout, nullptr);
                    sets.insert(sets.end(), desc.sets.begin(), desc.sets.end());
                }
                vkFreeDescriptorSets(this->m_device, this->m_descriptor_pool, (uint32_t)sets.size(), sets.data());
                this->m_descriptor_sets.clear();
            }
        }
    }
}
#include "bve_pch.h"
#include "vulkan_shader.h"
#include "vulkan_context.h"
#include "../../shader_compiler.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            vulkan_shader::vulkan_shader(ref<vulkan_object_factory> factory, const std::vector<fs::path>& sources) {
                this->m_device = nullptr;
                this->m_factory = factory;
                this->m_sources = sources;
                this->compile();
            }
            vulkan_shader::~vulkan_shader() {
                this->cleanup();
            }
            void vulkan_shader::reload() {
                this->cleanup();
                this->compile();
            }
            void vulkan_shader::bind() {
                // todo: stuff
            }
            void vulkan_shader::unbind() {
                // todo: stuff
            }
            void vulkan_shader::set_int(const std::string& name, int32_t value) {
                // todo: stuff
            }
            void vulkan_shader::set_float(const std::string& name, float value) {
                // todo: stuff
            }
            void vulkan_shader::set_ivec2(const std::string& name, const glm::ivec2& value) {
                // todo: stuff
            }
            void vulkan_shader::set_ivec3(const std::string& name, const glm::ivec3& value) {
                // todo: stuff
            }
            void vulkan_shader::set_ivec4(const std::string& name, const glm::ivec4& value) {
                // todo: stuff
            }
            void vulkan_shader::set_vec2(const std::string& name, const glm::vec2& value) {
                // todo: stuff
            }
            void vulkan_shader::set_vec3(const std::string& name, const glm::vec3& value) {
                // todo: stuff
            }
            void vulkan_shader::set_vec4(const std::string& name, const glm::vec4& value) {
                // todo: stuff
            }
            void vulkan_shader::set_mat4(const std::string& name, const glm::mat4& value) {
                // todo: stuff
            }
            int32_t vulkan_shader::get_int(const std::string& name) {
                return 0;
            }
            float vulkan_shader::get_float(const std::string& name) {
                return 0.f;
            }
            glm::ivec2 vulkan_shader::get_ivec2(const std::string& name) {
                return glm::ivec2(0);
            }
            glm::ivec3 vulkan_shader::get_ivec3(const std::string& name) {
                return glm::ivec3(0);
            }
            glm::ivec4 vulkan_shader::get_ivec4(const std::string& name) {
                return glm::ivec4(0);
            }
            glm::vec2 vulkan_shader::get_vec2(const std::string& name) {
                return glm::vec2(0.f);
            }
            glm::vec3 vulkan_shader::get_vec3(const std::string& name) {
                return glm::vec3(0.f);
            }
            glm::vec4 vulkan_shader::get_vec4(const std::string& name) {
                return glm::vec4(0.f);
            }
            glm::mat4 vulkan_shader::get_mat4(const std::string& name) {
                return glm::mat4(1.f);
            }
            void vulkan_shader::compile() {
                ref<vulkan_context> context_ = this->m_factory->get_current_context();
                this->m_device = context_->get_device();
                auto parser = shader_parser(shader_language::GLSL, shader_language::GLSL);
                for (const auto& source : this->m_sources) {
                    parser.parse(source);
                }
                std::vector<std::pair<shader_type, VkShaderModule>> shader_modules;
                for (shader_type type : parser.get_parsed_shader_types()) {
                    shader_modules.push_back({ type, this->compile_shader(type, parser) });
                }
                for (const auto& module : shader_modules) {
                    VkPipelineShaderStageCreateInfo create_info;
                    memset(&create_info, 0, sizeof(VkPipelineShaderStageCreateInfo));
                    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    switch (module.first) {
                    case shader_type::VERTEX:
                        create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                        break;
                    case shader_type::FRAGMENT:
                        create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                        break;
                    case shader_type::GEOMETRY:
                        create_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                        break;
                    default:
                        throw std::runtime_error("[vulkan shader] the given shader type is not supported yet");
                        break;
                    }
                    create_info.module = module.second;
                    create_info.pName = "main";
                    this->m_pipeline_create_info.push_back(create_info);
                }
            }
            VkShaderModule vulkan_shader::compile_shader(shader_type type, const shader_parser& parser) {
                std::string source = parser.get_shader(type);
                shader_compiler compiler;
                auto spirv = compiler.compile(source, shader_language::GLSL, type);
                VkShaderModuleCreateInfo create_info;
                memset(&create_info, 0, sizeof(VkShaderModuleCreateInfo));
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
                for (const auto& create_info : this->m_pipeline_create_info) {
                    vkDestroyShaderModule(this->m_device, create_info.module, nullptr);
                }
                this->m_pipeline_create_info.clear();
            }
        }
    }
}
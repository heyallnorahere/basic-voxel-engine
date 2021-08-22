#include "bve_pch.h"
#include "vulkan_shader.h"
namespace bve {
    namespace graphics {
        namespace vulkan {
            vulkan_shader::vulkan_shader(ref<vulkan_object_factory> factory, const std::vector<std::filesystem::path>& sources) {
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
                // todo: read, compile, and set up shader modules
            }
            void vulkan_shader::cleanup() {
                // todo: clean up shader modules
            }
        }
    }
}
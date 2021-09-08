#include "bve_pch.h"
#include "opengl_shader.h"
#include "shader_parser.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            opengl_shader::~opengl_shader() {
                this->destroy();
            }
            void opengl_shader::reload() {
                this->destroy();
                this->create();
            }
            void opengl_shader::bind() {
                glUseProgram(this->m_program);
            }
            void opengl_shader::unbind() {
                glUseProgram(0);
            }
            void opengl_shader::set_int(const std::string& name, int32_t value) {
                glUniform1i(this->get_location(name), value);
            }
            void opengl_shader::set_float(const std::string& name, float value) {
                glUniform1f(this->get_location(name), value);
            }
            void opengl_shader::set_ivec2(const std::string& name, const glm::ivec2& value) {
                glUniform2i(this->get_location(name), value.x, value.y);
            }
            void opengl_shader::set_ivec3(const std::string& name, const glm::ivec3& value) {
                glUniform3i(this->get_location(name), value.x, value.y, value.z);
            }
            void opengl_shader::set_ivec4(const std::string& name, const glm::ivec4& value) {
                glUniform4i(this->get_location(name), value.x, value.y, value.z, value.w);
            }
            void opengl_shader::set_vec2(const std::string& name, const glm::vec2& value) {
                glUniform2f(this->get_location(name), value.x, value.y);
            }
            void opengl_shader::set_vec3(const std::string& name, const glm::vec3& value) {
                glUniform3f(this->get_location(name), value.x, value.y, value.z);
            }
            void opengl_shader::set_vec4(const std::string& name, const glm::vec4& value) {
                glUniform4f(this->get_location(name), value.x, value.y, value.z, value.w);
            }
            void opengl_shader::set_mat4(const std::string& name, const glm::mat4& value) {
                glUniformMatrix4fv(this->get_location(name), 1, false, glm::value_ptr(value));
            }
            int32_t opengl_shader::get_int(const std::string& name) {
                int32_t value;
                glGetUniformiv(this->m_program, this->get_location(name), &value);
                return value;
            }
            float opengl_shader::get_float(const std::string& name) {
                float value;
                glGetUniformfv(this->m_program, this->get_location(name), &value);
                return value;
            }
            glm::ivec2 opengl_shader::get_ivec2(const std::string& name) {
                glm::ivec2 value;
                glGetUniformiv(this->m_program, this->get_location(name), &value.x);
                return value;
            }
            glm::ivec3 opengl_shader::get_ivec3(const std::string& name) {
                glm::ivec3 value;
                glGetUniformiv(this->m_program, this->get_location(name), &value.x);
                return value;
            }
            glm::ivec4 opengl_shader::get_ivec4(const std::string& name) {
                glm::ivec4 value;
                glGetUniformiv(this->m_program, this->get_location(name), &value.x);
                return value;
            }
            glm::vec2 opengl_shader::get_vec2(const std::string& name) {
                glm::vec2 value;
                glGetUniformfv(this->m_program, this->get_location(name), &value.x);
                return value;
            }
            glm::vec3 opengl_shader::get_vec3(const std::string& name) {
                glm::vec3 value;
                glGetUniformfv(this->m_program, this->get_location(name), &value.x);
                return value;
            }
            glm::vec4 opengl_shader::get_vec4(const std::string& name) {
                glm::vec4 value;
                glGetUniformfv(this->m_program, this->get_location(name), &value.x);
                return value;
            }
            glm::mat4 opengl_shader::get_mat4(const std::string& name) {
                glm::mat4 value;
                glGetUniformfv(this->m_program, this->get_location(name), &value[0][0]);
                return value;
            }
            opengl_shader::opengl_shader(const std::vector<fs::path>& sources) {
                this->m_sources = sources;
                this->create();
            }
            void opengl_shader::create() {
                auto parser = shader_parser(shader_language::GLSL, shader_language::GLSL);
                for (const auto& source : this->m_sources) {
                    parser.parse(source);
                }
                std::vector<GLuint> shaders;
                for (const auto& type : parser.get_parsed_shader_types()) {
                    GLenum shader_type_;
                    switch (type) {
                    case shader_type::VERTEX:
                        shader_type_ = GL_VERTEX_SHADER;
                        break;
                    case shader_type::FRAGMENT:
                        shader_type_ = GL_FRAGMENT_SHADER;
                        break;
                    case shader_type::GEOMETRY:
                        shader_type_ = GL_GEOMETRY_SHADER;
                        break;
                    default:
                        throw std::runtime_error("[opengl shader] invalid shader type");
                    }
                    std::string source = parser.get_shader(type);
                    auto path = parser.get_shader_path(type);
                    shaders.push_back(this->create_shader(source, shader_type_, path));
                }
                spdlog::info("[opengl shader] linking shader program...");
                this->m_program = glCreateProgram();
                for (GLuint shader_ : shaders) {
                    glAttachShader(this->m_program, shader_);
                }
                glLinkProgram(this->m_program);
                for (GLuint shader_ : shaders) {
                    glDeleteShader(shader_);
                }
                GLint status;
                glGetProgramiv(this->m_program, GL_LINK_STATUS, &status);
                if (!status) {
                    GLchar info_log[512];
                    glGetProgramInfoLog(this->m_program, 512, nullptr, info_log);
                    spdlog::warn("[opengl shader] error linking shader program: " + std::string(info_log));
                } else {
                    spdlog::info("[opengl shader] successfully linked shader program");
                }
            }
            void opengl_shader::destroy() {
                glDeleteProgram(this->m_program);
            }
            GLuint opengl_shader::create_shader(const std::string& source, GLenum type, std::optional<fs::path> path) {
                std::string shader_type;
                switch (type) {
                case GL_VERTEX_SHADER:
                    shader_type = "vertex";
                    break;
                case GL_FRAGMENT_SHADER:
                    shader_type = "fragment";
                    break;
                default:
                    shader_type = "other";
                    break;
                }
                spdlog::info("[opengl shader] compiling " + shader_type + " shader... (" + (path ? path->string() : "cannot determine path") + ")");
                const char* src = source.c_str();
                GLuint id = glCreateShader(type);
                glShaderSource(id, 1, &src, nullptr);
                glCompileShader(id);
                GLint status;
                glGetShaderiv(id, GL_COMPILE_STATUS, &status);
                if (!status) {
                    GLchar info_log[512];
                    glGetShaderInfoLog(id, 512, nullptr, info_log);
                    spdlog::warn("[opengl shader] error compiling " + shader_type + " shader: " + info_log);
                }
                else {
                    spdlog::info("[opengl shader] successfully compiled " + shader_type + " shader");
                }
                return id;
            }
            GLint opengl_shader::get_location(const std::string& name) {
                return glGetUniformLocation(this->m_program, name.c_str());
            }
        }
    }
}
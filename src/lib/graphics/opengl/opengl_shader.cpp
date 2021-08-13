#include "bve_pch.h"
#include "opengl_shader.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            std::string read_file(const std::string& path) {
                std::ifstream file(path);
                std::stringstream contents;
                std::string line;
                while (std::getline(file, line)) {
                    contents << line << '\n';
                }
                file.close();
                return contents.str();
            }
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
            opengl_shader::opengl_shader(const std::vector<opengl_shader_source>& sources) {
                this->m_sources = sources;
                this->create();
            }
            void opengl_shader::create() {
                std::vector<GLuint> shaders;
                for (const auto& source : this->m_sources) {
                    shaders.push_back(this->create_shader(source.path.string(), source.type));
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
                }
                else {
                    spdlog::info("[opengl shader] successfully linked shader program");
                }
            }
            void opengl_shader::destroy() {
                glDeleteProgram(this->m_program);
            }
            GLuint opengl_shader::create_shader(const std::string& path, GLenum type) {
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
                spdlog::info("[opengl shader] compiling " + shader_type + " shader... (" + path + ")");
                std::string source = read_file(path);
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
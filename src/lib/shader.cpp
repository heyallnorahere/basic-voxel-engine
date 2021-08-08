#include "bve_pch.h"
#include "shader.h"
namespace bve {
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
    static GLuint create_shader(const std::string& path, GLenum type) {
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
        spdlog::info("[bve::shader] compiling " + shader_type + " shader...");
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
            spdlog::error("[bve::shader] error compiling " + shader_type + " shader: " + info_log);
        } else {
            spdlog::info("[bve::shader] successfully compiled " + shader_type + " shader!");
        }
        return id;
    }
    std::shared_ptr<shader> shader::create(const std::initializer_list<intermediate_shader_source>& sources) {
        return std::shared_ptr<shader>(new shader(std::vector<intermediate_shader_source>(sources)));
    }
    shader::~shader() {
        this->destroy();
    }
    void shader::reload() {
        this->destroy();
        this->reload();
    }
    void shader::bind() {
        glUseProgram(this->m_program);
    }
    void shader::unbind() {
        glUseProgram(0);
    }
    shader::shader(const std::vector<intermediate_shader_source>& sources) {
        this->m_sources = sources;
        this->create();
    }
    void shader::create() {
        std::vector<GLuint> shaders;
        for (const auto& source : this->m_sources) {
            shaders.push_back(create_shader(source.path, source.type));
        }
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
            throw std::runtime_error("[bve::shader] error linking shader program: " + std::string(info_log));
        }
    }
    void shader::destroy() {
        glDeleteProgram(this->m_program);
    }
}
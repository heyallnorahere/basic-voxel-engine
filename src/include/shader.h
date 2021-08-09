#pragma once
namespace bve {
    class shader {
    public:
        struct intermediate_shader_source {
            std::string path;
            GLenum type;
        };
        static std::shared_ptr<shader> create(const std::initializer_list<intermediate_shader_source>& sources);
        ~shader();
        void reload();
        void bind();
        void unbind();
        template<typename T> void set_uniform(const std::string& name, const T& value);
    private:
        shader(const std::vector<intermediate_shader_source>& sources);
        void create();
        void destroy();
        GLuint create_shader(const std::string& path, GLenum type);
        GLint get_location(const std::string& name);
        GLuint m_program;
        std::vector<intermediate_shader_source> m_sources;
    };
    template<> inline void shader::set_uniform<GLint>(const std::string& name, const GLint& value) {
        glUniform1i(this->get_location(name), value);
    }
    template<> inline void shader::set_uniform<GLfloat>(const std::string& name, const GLfloat& value) {
        glUniform1f(this->get_location(name), value);
    }
    template<> inline void shader::set_uniform<glm::vec3>(const std::string& name, const glm::vec3& value) {
        glUniform3f(this->get_location(name), value.x, value.y, value.z);
    }
    template<> inline void shader::set_uniform<glm::vec4>(const std::string& name, const glm::vec4& value) {
        glUniform4f(this->get_location(name), value.x, value.y, value.z, value.w);
    }
    template<> inline void shader::set_uniform<glm::mat4>(const std::string& name, const glm::mat4& value) {
        glUniformMatrix4fv(this->get_location(name), 1, false, glm::value_ptr(value));
    }
    template<typename T> inline void shader::set_uniform(const std::string& name, const T& value) {
        throw std::runtime_error("[shader] no such uniform implementation!");
    }
}

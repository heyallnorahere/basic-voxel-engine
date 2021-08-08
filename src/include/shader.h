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
        GLuint m_program;
        std::vector<intermediate_shader_source> m_sources;
    };
    template<> inline void shader::set_uniform<GLint>(const std::string& name, const GLint& value) {
        glUniform1i(glGetUniformLocation(this->m_program, name.c_str()), value);
    }
    template<typename T> inline void shader::set_uniform(const std::string& name, const T& value) {
        throw std::runtime_error("No such uniform implementation!");
    }
}

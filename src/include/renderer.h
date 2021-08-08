#pragma once
namespace bve {
    enum class vertex_attribute_type {
        FLOAT,
        INT,
        VEC2,
        IVEC2,
        VEC3,
        IVEC3,
        VEC4,
        IVEC4,
        MAT4
    };
    struct vertex_attribute {
        size_t stride, offset;
        vertex_attribute_type type;
        bool normalize;
    };
    struct command_list;
    class shader;
    class renderer {
    public:
        struct stats {
            uint32_t rendercalls;
        };
        renderer() { }
        renderer(const renderer&) = delete;
        renderer& operator=(const renderer&) = delete;
        command_list* create_command_list();
        void destroy_command_list(command_list* cmdlist);
        void add_mesh(command_list* cmdlist, GLuint vertex_buffer, GLuint index_buffer, size_t index_count);
        void close_command_list(command_list* cmdlist, const std::vector<vertex_attribute>& attributes);
        void render(command_list* cmdlist, std::shared_ptr<shader> shader_);
        void set_camera_data(glm::vec3 position, glm::vec3 direction, float aspect_ratio, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
    private:
        glm::mat4 m_projection, m_view;
    };
}
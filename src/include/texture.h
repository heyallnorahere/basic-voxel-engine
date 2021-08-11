#pragma once
namespace bve {
    struct texture_settings {
        GLenum min_filter = 0;
        GLenum mag_filter = 0;
        GLenum wrap_s = 0;
        GLenum wrap_t = 0;
        GLenum target = 0;
        GLenum format = 0;
    };
    class texture {
    public:
        texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const texture_settings& settings);
        texture(const std::string& path, const texture_settings& settings);
        ~texture();
        texture(const texture&) = delete;
        texture& operator=(const texture&) = delete;
        void bind(GLint slot = 0) const;
    private:
        void create(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const texture_settings& settings);
        void create(const std::string& path, const texture_settings& settings);
        GLuint m_id;
        GLenum m_target;
    };
}
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
    class texture : public ref_counted {
    public:
        static std::vector<uint8_t> load_image(const std::filesystem::path& path, int32_t& width, int32_t& height, int32_t& channels);
        texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const texture_settings& settings);
        texture(const std::string& path, const texture_settings& settings);
        ~texture();
        texture(const texture&) = delete;
        texture& operator=(const texture&) = delete;
        void bind(GLint slot = 0) const;
        glm::ivec2 get_size() const;
        int32_t get_channels() const;
        GLuint get_id() const;
    private:
        void create(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const texture_settings& settings);
        void create(const std::string& path, const texture_settings& settings);
        GLuint m_id;
        GLenum m_target;
        glm::ivec2 m_size;
        int32_t m_channels;
    };
}
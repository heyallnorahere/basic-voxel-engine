#include "bve_pch.h"
#include "texture.h"
// todo: add check to see if stb_image was already implemented
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace bve {
    std::vector<uint8_t> texture::load_image(const std::filesystem::path& path, int32_t& width, int32_t& height, int32_t& channels) {
        std::string string_path = path.string();
        uint8_t* data_pointer = stbi_load(string_path.c_str(), &width, &height, &channels, 0);
        if (!data_pointer) {
            throw std::runtime_error("[texture] could not open file: " + string_path);
        }
        size_t buffer_size = (size_t)width * height * channels;
        std::vector<uint8_t> data(buffer_size);
        std::copy(data_pointer, data_pointer + buffer_size, data.begin());
        stbi_image_free(data_pointer);
        return data;
    }
    texture::texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const texture_settings& settings) {
        this->create(data, width, height, channels, settings);
    }
    texture::texture(const std::string& path, const texture_settings& settings) {
        this->create(path, settings);
    }
    texture::~texture() {
        glDeleteTextures(1, &this->m_id);
    }
    void texture::bind(GLint slot) const {
        GLint max_texture_image_units;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units);
        if (slot >= max_texture_image_units) {
            throw std::runtime_error("[texture] attempted to bind a texture to a nonexistend texture slot");
        }
        glActiveTexture(GL_TEXTURE0 + (GLenum)slot);
        glBindTexture(this->m_target, this->m_id);
    }
    glm::ivec2 texture::get_size() const {
        return this->m_size;
    }
    int32_t texture::get_channels() const {
        return this->m_channels;
    }
    GLuint texture::get_id() const {
        return this->m_id;
    }
    void texture::create(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const texture_settings& settings) {
        this->m_target = settings.target != 0 ? settings.target : GL_TEXTURE_2D;
        glGenTextures(1, &this->m_id);
        glBindTexture(this->m_target, this->m_id);
#define texture_parameter(name, field, default_value) glTexParameteri(this->m_target, name, settings.field != 0 ? settings.field : default_value)
        texture_parameter(GL_TEXTURE_MIN_FILTER, min_filter, GL_LINEAR);
        texture_parameter(GL_TEXTURE_MAG_FILTER, mag_filter, GL_LINEAR);
        texture_parameter(GL_TEXTURE_WRAP_S, wrap_s, GL_CLAMP_TO_EDGE);
        texture_parameter(GL_TEXTURE_WRAP_T, wrap_t, GL_CLAMP_TO_EDGE);
#undef texture_parameter
        GLint internal_format;
        switch (channels) {
        case 1:
            internal_format = GL_R8;
            break;
        case 2:
            internal_format = GL_RG;
            break;
        case 3:
            internal_format = GL_RGB;
            break;
        case 4:
            internal_format = GL_RGBA;
            break;
        default:
            throw std::runtime_error("[texture] invalid channel count");
            break;
        }
        GLenum format = settings.format != 0 ? settings.format : (GLenum)internal_format;
        glTexImage2D(this->m_target, 0, internal_format, (GLsizei)width, (GLsizei)height, 0, format, GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(this->m_target);
        this->m_size = glm::ivec2(width, height);
        this->m_channels = channels;
    }
    void texture::create(const std::string& path, const texture_settings& settings) {
        int32_t width, height, channels;
        std::vector<uint8_t> data = load_image(path, width, height, channels);
        this->create(data, width, height, channels, settings);
    }
}
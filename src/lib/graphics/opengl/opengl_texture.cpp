#include "bve_pch.h"
#include "opengl_texture.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            opengl_texture::opengl_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const opengl_texture_settings& settings) {
                this->create(data, width, height, channels, settings);
            }
            opengl_texture::~opengl_texture() {
                glDeleteTextures(1, &this->m_id);
            }
            void opengl_texture::bind(GLint slot) {
                GLint max_texture_image_units;
                glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units);
                if (slot >= max_texture_image_units) {
                    throw std::runtime_error("[opengl texture] attempted to bind a texture to a nonexistend texture slot");
                }
                glActiveTexture(GL_TEXTURE0 + (GLenum)slot);
                glBindTexture(this->m_target, this->m_id);
            }
            glm::ivec2 opengl_texture::get_size() {
                return this->m_size;
            }
            int32_t opengl_texture::get_channels() {
                return this->m_channels;
            }
            void* opengl_texture::get_id() {
                return (void*)(size_t)this->m_id;
            }
            void opengl_texture::create(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const opengl_texture_settings& settings) {
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
                GLenum format;
                if (channels == 3) {
                    internal_format = GL_RGB8;
                    format = GL_RGB;
                } else if (channels == 4) {
                    internal_format = GL_RGBA8;
                    format = GL_RGBA;
                } else {
                    throw std::runtime_error("[opengl texture] invalid channel count");
                }
                glTexImage2D(this->m_target, 0, internal_format, (GLsizei)width, (GLsizei)height, 0, format, GL_UNSIGNED_BYTE, data.data());
                glGenerateMipmap(this->m_target);
                this->m_size = glm::ivec2(width, height);
                this->m_channels = channels;
            }
        }
    }
}
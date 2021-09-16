#include "bve_pch.h"
#include "opengl_texture.h"
#include "opengl_context.h"
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
                    throw std::runtime_error("[opengl texture] attempted to bind a texture to a nonexistent texture slot");
                }
                glBindTextureUnit((GLuint)slot, this->m_id);
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
                if (opengl_context::get_version() < 4.5) {
                    throw std::runtime_error("[opengl texture] OpenGL functions used require version 4.5");
                }
                glCreateTextures(this->m_target, 1, &this->m_id);
                GLenum internal_format = 0, format = settings.format;
                switch (channels) {
                case 3:
                    internal_format = GL_RGB8;
                    if (!format) {
                        format = GL_RGB;
                    }
                    break;
                case 4:
                    internal_format = GL_RGBA8;
                    if (!format) {
                        format = GL_RGBA;
                    }
                    break;
                default:
                    throw std::runtime_error("[opengl texture] invalid channel count");
                    break;
                }
                glTextureStorage2D(this->m_id, 1, internal_format, (GLsizei)width, (GLsizei)height);
#define texture_parameter(name, field, default_value) glTextureParameteri(this->m_id, name, settings.field != 0 ? settings.field : default_value)
                texture_parameter(GL_TEXTURE_MIN_FILTER, min_filter, GL_LINEAR);
                texture_parameter(GL_TEXTURE_MAG_FILTER, mag_filter, GL_NEAREST);
                texture_parameter(GL_TEXTURE_WRAP_S, wrap_s, GL_REPEAT);
                texture_parameter(GL_TEXTURE_WRAP_T, wrap_t, GL_REPEAT);
#undef texture_parameter
                glTextureSubImage2D(this->m_id, 0, 0, 0, (GLsizei)width, (GLsizei)height, format, GL_UNSIGNED_BYTE, data.data());
                this->m_size = glm::ivec2(width, height);
                this->m_channels = channels;
            }
        }
    }
}
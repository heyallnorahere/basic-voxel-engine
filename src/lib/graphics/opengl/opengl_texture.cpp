#include "bve_pch.h"
#include "opengl_texture.h"
#include "opengl_context.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            opengl_texture::opengl_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) {
                this->create(data, width, height, channels);
            }
            opengl_texture::~opengl_texture() {
                glDeleteTextures(1, &this->m_id);
            }
            void opengl_texture::bind(uint32_t slot) {
                int32_t max_texture_image_units;
                glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units);
                if (slot >= (uint32_t)max_texture_image_units) {
                    throw std::runtime_error("[opengl texture] attempted to bind a texture to a nonexistent texture slot");
                }
                glBindTextureUnit(slot, this->m_id);
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
            void opengl_texture::create(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) {
                if (opengl_context::get_version() < 4.5) {
                    throw std::runtime_error("[opengl texture] OpenGL functions used require version 4.5");
                }
                GLenum internal_format, format;
                switch (channels) {
                case 3:
                    internal_format = GL_RGB8;
                    format = GL_RGB;
                    break;
                case 4:
                    internal_format = GL_RGBA8;
                    format = GL_RGBA;
                    break;
                default:
                    throw std::runtime_error("[opengl texture] invalid channel count");
                    break;
                }
                glCreateTextures(GL_TEXTURE_2D, 1, &this->m_id);
                glTextureStorage2D(this->m_id, 1, internal_format, (GLsizei)width, (GLsizei)height);
                glTextureParameteri(this->m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(this->m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(this->m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTextureParameteri(this->m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTextureSubImage2D(this->m_id, 0, 0, 0, (GLsizei)width, (GLsizei)height, format, GL_UNSIGNED_BYTE, data.data());
                glGenerateTextureMipmap(this->m_id);
                this->m_size = glm::ivec2(width, height);
                this->m_channels = channels;
            }
        }
    }
}
#pragma once
#include "graphics/texture.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            struct opengl_texture_settings {
                GLenum min_filter = 0;
                GLenum mag_filter = 0;
                GLenum wrap_s = 0;
                GLenum wrap_t = 0;
                GLenum target = 0;
                GLenum format = 0;
            };
            class opengl_texture : public texture {
            public:
                opengl_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const opengl_texture_settings& settings);
                virtual ~opengl_texture() override;
                virtual void bind(GLint slot) override;
                virtual glm::ivec2 get_size() override;
                virtual int32_t get_channels() override;
                virtual void* get_id() override;
            private:
                void create(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels, const opengl_texture_settings& settings);
                GLuint m_id;
                GLenum m_target;
                glm::ivec2 m_size;
                int32_t m_channels;
            };
        }
    }
}
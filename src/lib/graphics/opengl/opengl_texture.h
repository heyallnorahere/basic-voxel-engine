#pragma once
#include "graphics/texture.h"
namespace bve {
    namespace graphics {
        namespace opengl {
            class opengl_texture : public texture {
            public:
                opengl_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels);
                virtual ~opengl_texture() override;
                virtual void bind(uint32_t slot) override;
                virtual glm::ivec2 get_size() override;
                virtual int32_t get_channels() override;
                virtual void* get_id() override;
            private:
                void create(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels);
                uint32_t m_id;
                glm::ivec2 m_size;
                int32_t m_channels;
            };
        }
    }
}
#pragma once
#include "buffer.h"
#include "context.h"
#include "vao.h"
#include "shader.h"
#include "texture.h"
#include "uniform_buffer.h"
namespace bve {
    namespace graphics {
        enum class graphics_api {
            OPENGL,
            VULKAN
        };
        class object_factory : public ref_counted {
        public:
            static ref<object_factory> create(graphics_api api);
            object_factory() = default;
            virtual ~object_factory() = default;
            object_factory(const object_factory&) = delete;
            object_factory& operator=(const object_factory&) = delete;
            virtual ref<vao> create_vao() = 0;
            template<typename T> ref<buffer> create_vbo(const std::vector<T>& data) {
                return this->create_vbo(data.data(), data.size() * sizeof(T));
            }
            virtual ref<buffer> create_vbo(const void* data, size_t size) = 0;
            virtual ref<buffer> create_ebo(const std::vector<uint32_t>& data) = 0;
            virtual ref<context> create_context() = 0;
            virtual ref<shader> create_shader(const std::vector<fs::path>& sources) = 0;
            ref<texture> create_texture(const fs::path& path) {
                int32_t width, height, channels;
                std::vector<uint8_t> data;
                if (!texture::load_image(path, data, width, height, channels)) {
                    throw std::runtime_error("[object factory] could not load image from file: " + path.string());
                }
                return this->create_texture(data, width, height, channels);
            }
            virtual ref<texture> create_texture(const std::vector<uint8_t>& data, int32_t width, int32_t height, int32_t channels) = 0;
            virtual ref<uniform_buffer> create_uniform_buffer(size_t size, uint32_t binding) = 0;
            graphics_api get_graphics_api() { return this->m_graphics_api; }
        private:
            graphics_api m_graphics_api;
        };
    }
}
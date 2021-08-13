#include "bve_pch.h"
#include "graphics/object_factory.h"
#include "opengl/opengl_object_factory.h"
// todo: add check to see if stb_image was already implemented
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace bve {
    namespace graphics {
        ref<object_factory> object_factory::create(graphics_api api) {
            switch (api) {
            case graphics_api::OPENGL:
                return ref<opengl::opengl_object_factory>::create();
                break;
            default:
                return nullptr;
            }
        }
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
    }
}
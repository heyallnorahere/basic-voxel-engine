#include "bve_pch.h"
#include "graphics/object_factory.h"
#include "opengl/opengl_object_factory.h"
#include "vulkan/vulkan_object_factory.h"
#include "../shader_compiler.h"
// todo: add check to see if stb_image was already implemented
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace bve {
    namespace graphics {
        ref<object_factory> object_factory::create(graphics_api api) {
            ref<object_factory> factory;
            switch (api) {
            case graphics_api::OPENGL:
                factory = ref<opengl::opengl_object_factory>::create();
                break;
            case graphics_api::VULKAN:
                factory = ref<vulkan::vulkan_object_factory>::create();
                break;
            default:
                return nullptr;
            }
            factory->m_graphics_api = api;
            return factory;
        }
        bool texture::load_image(const fs::path& path, std::vector<uint8_t>& data, int32_t& width, int32_t& height, int32_t& channels) {
            std::string string_path = path.string();
            uint8_t* data_pointer = stbi_load(string_path.c_str(), &width, &height, &channels, 4);
            if (!data_pointer) {
                spdlog::warn("[texture] could not open file : " + string_path);
                return false;
            }
            size_t buffer_size = (size_t)width * height * channels;
            data.resize(buffer_size);
            std::copy(data_pointer, data_pointer + buffer_size, data.begin());
            stbi_image_free(data_pointer);
            return true;
        }
        size_t struct_data::find_offset(const std::string& field_name) {
            size_t separator_pos = field_name.find('.');
            std::string name, subname;
            if (separator_pos != std::string::npos) {
                name = field_name.substr(0, separator_pos);
                subname = field_name.substr(separator_pos + 1);
                if (subname.empty()) {
                    throw std::runtime_error("[shader compiler] invalid field name");
                }
            } else {
                name = field_name;
            }
            if (this->fields.find(name) == this->fields.end()) {
                throw std::runtime_error("[shader compiler] " + name + " is not the name of a field");
            }
            const auto& field = this->fields[name];
            size_t offset = field.offset;
            if (subname.empty()) {
                return offset;
            } else {
                return offset + field.type->find_offset(subname);
            }
        }
        void shader::reflect(shader_type type, const std::vector<uint32_t>& spirv) {
            shader_compiler compiler;
            compiler.reflect(spirv, this->m_reflection_data[type]);
        }
    }
}
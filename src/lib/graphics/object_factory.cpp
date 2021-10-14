#include "bve_pch.h"
#include "graphics/object_factory.h"
#include "opengl/opengl_object_factory.h"
#include "vulkan/vulkan_object_factory.h"
#ifdef BVE_DX12_BACKEND_ENABLED
#include "directx12/dx12_object_factory.h"
#endif
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
#ifdef BVE_DX12_BACKEND_ENABLED
            case graphics_api::DIRECTX12:
                factory = ref<dx12::dx12_object_factory>::create();
                break;
#endif
            default:
                throw std::runtime_error("[object factory] the requested graphics api is not supported on this platform");
            }
            factory->m_graphics_api = api;
            return factory;
        }
        bool texture::load_image(const fs::path& path, std::vector<uint8_t>& data, int32_t& width, int32_t& height, int32_t& channels) {
            std::string string_path = path.string();
            uint8_t* data_pointer = stbi_load(string_path.c_str(), &width, &height, &channels, 0);
            if (!data_pointer) {
                spdlog::warn("[texture] could not open file: " + string_path);
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
            int32_t index = 0;
            size_t open_bracket = name.find('[');
            if (open_bracket != std::string::npos) {
                size_t close_bracket = name.find(']');
                if (close_bracket <= open_bracket + 1 || close_bracket >= name.length() || close_bracket < name.length() - 1) {
                    throw std::runtime_error("[shader compiler] invalid index operator call");
                }
                size_t index_start = open_bracket + 1;
                std::string index_string = name.substr(index_start, close_bracket - index_start);
                name = name.substr(0, open_bracket);
                index = atoi(index_string.c_str());
            }
            if (this->fields.find(name) == this->fields.end()) {
                throw std::runtime_error("[shader compiler] " + name + " is not the name of a field");
            }
            const auto& field = this->fields[name];
            if (index > 0 && field.type->array_size <= 1) {
                throw std::runtime_error("[shader compiler] attempted to index into a non-array field");
            }
            size_t offset = field.offset + (index * field.type->array_stride);
            if (subname.empty()) {
                return offset;
            } else {
                return offset + field.type->find_offset(subname);
            }
        }
        uint32_t reflection_output::get_descriptor_set_count() const {
            uint32_t descriptor_set_count = 0;
            for (const auto& [binding, resource_info] : this->uniform_buffers) {
                uint32_t new_size = resource_info.descriptor_set + 1;
                if (descriptor_set_count < new_size) {
                    descriptor_set_count = new_size;
                }
            }
            for (const auto& [binding, resource_info] : this->sampled_images) {
                uint32_t new_size = resource_info.descriptor_set + 1;
                if (descriptor_set_count < new_size) {
                    descriptor_set_count = new_size;
                }
            }
            return descriptor_set_count;
        }
        void shader::reflect(shader_type type, const std::vector<uint32_t>& spirv) {
            shader_compiler compiler;
            compiler.reflect(spirv, type, this->m_reflection_data);
        }
    }
}
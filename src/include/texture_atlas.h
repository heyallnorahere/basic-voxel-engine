#pragma once
#include "registry.h"
#include "graphics/shader.h"
#include "texture.h"
namespace bve {
    class asset_manager;
    class texture_atlas : public ref_counted {
    public:
        texture_atlas(const texture_atlas&) = delete;
        texture_atlas& operator=(const texture_atlas&) = delete;
        void set_uniform(ref<graphics::shader> shader_, const std::string& uniform_name, GLint texture_slot = 0);
        glm::ivec2 get_texture_size();
        glm::ivec2 get_atlas_size();
        ref<texture> get_texture();
    private:
        struct texture_data {
            std::vector<uint8_t> data;
            int32_t width, height, channels;
        };
        texture_atlas(const std::vector<std::pair<namespaced_name, texture_data>>& textures);
        ref<texture> m_texture;
        glm::ivec2 m_texture_size, m_atlas_size;
        std::unordered_map<namespaced_name, std::pair<glm::ivec2, glm::ivec2>, namespaced_name::hash_function> m_texture_dimensions;
        friend class asset_manager;
    };
}
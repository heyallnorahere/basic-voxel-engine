#pragma once
#include "graphics/object_factory.h"
namespace bve {
    class asset_manager;
    class texture_atlas : public ref_counted {
    public:
        struct texture_dimensions_t {
            glm::ivec2 grid_position, texture_dimensions;
        };
        struct uniform_data {
            int32_t image;
            glm::ivec2 texture_size, grid_size;
            texture_dimensions_t texture_dimensions[64];
        };
        texture_atlas(const texture_atlas&) = delete;
        texture_atlas& operator=(const texture_atlas&) = delete;

        
        uniform_data get_uniform_data();
        glm::ivec2 get_texture_size();
        glm::ivec2 get_atlas_size();
        ref<graphics::texture> get_texture();
        std::vector<size_t> get_included_block_ids();

    private:
        struct texture_data {
            std::vector<uint8_t> data;
            int32_t width, height, channels;
        };
        texture_atlas(const std::vector<std::pair<size_t, texture_data>>& textures, ref<graphics::object_factory> object_factory);
        ref<graphics::texture> m_texture;
        glm::ivec2 m_texture_size, m_atlas_size;
        std::unordered_map<size_t, std::pair<glm::ivec2, glm::ivec2>> m_texture_dimensions;
        friend class asset_manager;
    };
}
#pragma once
#include "texture.h"
#include "registry.h"
namespace bve {
    class asset_manager;
    class texture_atlas {
    public:
        texture_atlas(const texture_atlas&) = delete;
        texture_atlas& operator=(const texture_atlas&) = delete;
        void bind(GLint slot = 0);
        glm::vec2 find_offset(const namespaced_name& block_name);
        glm::vec2 find_offset(size_t block_type_index);
        glm::vec2 get_uv_scale();
    private:
        struct texture_data {
            std::vector<uint8_t> data;
            int32_t width, height, channels;
        };
        texture_atlas(const std::vector<std::pair<namespaced_name, texture_data>>& textures);
        std::shared_ptr<texture> m_texture;
        glm::ivec2 m_texture_size, m_block_count;
        std::unordered_map<namespaced_name, glm::vec2, namespaced_name::hash_function> m_offsets;
        friend class asset_manager;
    };
    class asset_manager {
    public:
        static asset_manager& get();
        asset_manager(const asset_manager&) = delete;
        asset_manager& operator=(const asset_manager&) = delete;
        void reload(const std::vector<std::filesystem::path>& resource_packs);
        std::filesystem::path get_asset_path(const std::string& asset_name);
        std::shared_ptr<texture_atlas> create_texture_atlas();
    private:
        asset_manager() = default;
        std::filesystem::path m_asset_directory;
    };
}
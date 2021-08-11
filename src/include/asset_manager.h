#pragma once
#include "texture.h"
#include "registry.h"
#include "world.h"
#include "shader.h"
namespace bve {
    class asset_manager;
    class texture_atlas {
    public:
        texture_atlas(const texture_atlas&) = delete;
        texture_atlas& operator=(const texture_atlas&) = delete;
        void set_uniform(std::shared_ptr<shader> shader_, const std::string& uniform_name, GLint texture_slot = 0);
        glm::ivec2 get_texture_size();
        glm::ivec2 get_atlas_size();
        std::shared_ptr<texture> get_texture();
    private:
        struct texture_data {
            std::vector<uint8_t> data;
            int32_t width, height, channels;
        };
        texture_atlas(const std::vector<std::pair<namespaced_name, texture_data>>& textures);
        std::shared_ptr<texture> m_texture;
        glm::ivec2 m_texture_size, m_atlas_size;
        std::unordered_map<namespaced_name, std::pair<glm::ivec2, glm::ivec2>, namespaced_name::hash_function> m_texture_dimensions;
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
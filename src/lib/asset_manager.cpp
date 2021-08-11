#include "bve_pch.h"
#include "asset_manager.h"
#include "block.h"
#include <stb_image.h>
namespace bve {
    void texture_atlas::bind(GLint slot) {
        this->m_texture->bind(slot);
    }
    glm::vec2 texture_atlas::find_offset(const namespaced_name& block_name) {
        if (this->m_offsets.find(block_name) != this->m_offsets.end()) {
            throw std::runtime_error("[texture atlas] no texture was found matching that name");
        }
        return this->m_offsets[block_name];
    }
    glm::vec2 texture_atlas::find_offset(size_t block_type_index) {
        object_register<block>& block_register = registry::get().get_register<block>();
        std::optional<namespaced_name> name = block_register.get_name(block_type_index);
        if (!name) {
            throw std::runtime_error("[texture atlas] could not find the name of the given index in the block object register");
        }
        return this->find_offset(*name);
    }
    glm::vec2 texture_atlas::get_uv_scale() {
        return 1.f / glm::vec2(this->m_block_count);
    }
    texture_atlas::texture_atlas(const std::vector<std::pair<namespaced_name, texture_data>>& textures) {
        // i know this code is dubious but its the best i could think of
        constexpr int32_t max_block_width = 10;
        int32_t texture_count = (int32_t)textures.size();
        if (texture_count <= max_block_width) {
            this->m_block_count = glm::ivec2(texture_count, 1);
        } else {
            int32_t remainder_ = texture_count % max_block_width;
            this->m_block_count.x = max_block_width;
            this->m_block_count.y = ((texture_count - remainder_) / max_block_width) + (remainder_ > 0 ? 1 : 0);
        }
        this->m_texture_size = glm::ivec2(0);
        std::vector<glm::vec2*> offset_pointers;
        for (const auto& pair : textures) {
            if (pair.second.channels != 4) {
                throw std::runtime_error("[texture atlas] textures must be in RGBA format");
            }
            // STILL WORKING ON THIS
        }
    }
    asset_manager& asset_manager::get() {
        static asset_manager instance;
        return instance;
    }
    void asset_manager::reload(const std::vector<std::filesystem::path>& resource_packs) {
        namespace fs = std::filesystem;
        this->m_asset_directory = fs::temp_directory_path() / "basic-voxel-engine";
        fs::create_directory(this->m_asset_directory);
        const fs::copy_options options =
            fs::copy_options::update_existing |
            fs::copy_options::recursive;
        for (const auto& path : resource_packs) {
            fs::copy(path, this->m_asset_directory, options);
        }
    }
    std::filesystem::path asset_manager::get_asset_path(const std::string& asset_name) {
        constexpr char separator = ':';
        std::filesystem::path asset_path = this->m_asset_directory;
        if (asset_name.find(separator) == std::string::npos) {
            this->m_asset_directory /= asset_name;
        } else {
            for (size_t position = asset_name.find(separator), last_position = 0, substring_position = 0; last_position != std::string::npos; last_position = position, substring_position = last_position + 1, position = asset_name.find(separator, position + 1)) {
                asset_path /= asset_name.substr(substring_position, position - substring_position);
            }
        }
        return asset_path;
    }
    std::shared_ptr<texture_atlas> asset_manager::create_texture_atlas() {
        object_register<block>& block_register = registry::get().get_register<block>();
        std::vector<namespaced_name> register_names = block_register.get_names();
        using texture_data = texture_atlas::texture_data;
        std::vector<std::pair<namespaced_name, texture_data>> textures;
        for (const auto& name : register_names) {
            std::shared_ptr<block> block_ = block_register[name];
            auto path = this->get_asset_path("block:" + name.get_full_name() + ".png"); // single texture per block, for now
            texture_data data;
            try {
                data.data = texture::load_image(path, data.width, data.height, data.channels);
            } catch (std::runtime_error) {
                continue; // we dont know if the block will even be rendered - so we pass
            }
            textures.push_back({ name, data });
        }
        return std::shared_ptr<texture_atlas>(new texture_atlas(textures));
    }
}
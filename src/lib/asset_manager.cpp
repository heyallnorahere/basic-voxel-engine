#include "bve_pch.h"
#include "asset_manager.h"
#include "block.h"
namespace bve {
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
    ref<texture_atlas> asset_manager::create_texture_atlas(ref<graphics::object_factory> object_factory) {
        object_register<block>& block_register = registry::get().get_register<block>();
        std::vector<namespaced_name> register_names = block_register.get_names();
        using texture_data = texture_atlas::texture_data;
        std::vector<std::pair<namespaced_name, texture_data>> textures;
        for (const auto& name : register_names) {
            ref<block> block_ = block_register[name];
            auto path = this->get_asset_path("block:" + name.get_full_name() + ".png"); // single texture per block, for now
            texture_data data;
            try {
                data.data = graphics::texture::load_image(path, data.width, data.height, data.channels);
            } catch (const std::runtime_error&) {
                continue; // we dont know if the block will even be rendered - so we pass
            }
            textures.push_back({ name, data });
        }
        return ref<texture_atlas>(new texture_atlas(textures, object_factory));
    }
}
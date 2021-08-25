#pragma once
#include "texture_atlas.h"
namespace bve {
    // asset_manager is a global object that keeps track of all data
    // loaded from files such as textures.
    class asset_manager {
    public:
        // Get the global instance
        static asset_manager& get();

        asset_manager(const asset_manager&) = delete;
        asset_manager& operator=(const asset_manager&) = delete;

        // Reload all of the data from the resource_pack directory.
        void reload(const std::vector<std::filesystem::path>& resource_packs);

        // Get the filesystem path for an asset based on the name of the asset
        std::filesystem::path get_asset_path(const std::string& asset_name);

        // Create a texture atlas
        ref<texture_atlas> create_texture_atlas(ref<graphics::object_factory> object_factory);
    private:
        asset_manager() = default;
        std::filesystem::path m_asset_directory;
    };
}
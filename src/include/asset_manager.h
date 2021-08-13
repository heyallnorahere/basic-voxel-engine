#pragma once
#include "texture_atlas.h"
namespace bve {
    class asset_manager {
    public:
        static asset_manager& get();
        asset_manager(const asset_manager&) = delete;
        asset_manager& operator=(const asset_manager&) = delete;
        void reload(const std::vector<std::filesystem::path>& resource_packs);
        std::filesystem::path get_asset_path(const std::string& asset_name);
        ref<texture_atlas> create_texture_atlas(ref<graphics::object_factory> object_factory);
    private:
        asset_manager() = default;
        std::filesystem::path m_asset_directory;
    };
}
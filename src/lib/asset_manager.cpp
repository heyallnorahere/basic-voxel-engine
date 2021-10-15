#include "bve_pch.h"
#include "asset_manager.h"
#include "code_host.h"
namespace bve {
    asset_manager& asset_manager::get() {
        static asset_manager instance;
        return instance;
    }
    void asset_manager::reload(const std::vector<fs::path>& resource_packs) {

        // Copy all of the resource packs into a temp directory. This allows resource packs 
        // to overwrite resources in packs that come before them.
        this->m_asset_directory = fs::temp_directory_path() / "basic-voxel-engine";
        fs::create_directory(this->m_asset_directory);
        const fs::copy_options options =
            fs::copy_options::update_existing |
            fs::copy_options::recursive;
        for (const auto& path : resource_packs) {
            fs::copy(path, this->m_asset_directory, options);
        }
    }
    fs::path asset_manager::get_asset_path(const std::string& asset_name) {
        constexpr char separator = ':';
        fs::path asset_path = this->m_asset_directory;
        
        // If there is no namespace on the asset then just use the name directly.
        if (asset_name.find(separator) == std::string::npos) {
            this->m_asset_directory /= asset_name;
        } else {
            // sorry for the spaghetti code lmao
            // Split the resource name into segments and add them as components of the path.
            for (size_t position = asset_name.find(separator), last_position = 0, substring_position = 0; 
                last_position != std::string::npos; 
                last_position = position, substring_position = last_position + 1, 
                  position = asset_name.find(separator, position + 1)) {
                asset_path /= asset_name.substr(substring_position, position - substring_position);
            }
        }
        return asset_path;
    }
    struct namespaced_name {
        MonoString* namespace_name;
        MonoString* local_name;
    };
    ref<texture_atlas> asset_manager::create_texture_atlas(ref<graphics::object_factory> object_factory) {
        using texture_data = texture_atlas::texture_data;

        // For each type of block, look for an image based on the block name. 
        // Load and collect all of those images and use them to initialize a texture atlas.
        auto host = code_host::current();
        auto block_class = host->find_class("BasicVoxelEngine.Block");
        auto block_type = managed::type::get_type(block_class)->get_object();
        auto helpers_class = host->find_class("BasicVoxelEngine.Helpers");
        auto getregister = helpers_class->get_method("*:GetRegister");
        auto register_object = managed::class_::invoke(getregister, block_type);
        auto register_class = managed::class_::get_class(register_object);
        auto count_property = register_class->get_property("Count");
        int32_t count = register_object->get(count_property)->unbox<int32_t>();
        auto getnamespacedname = register_class->get_method("*:GetNamespacedName(int)");
        std::unordered_map<size_t, std::string> register_names;
        MonoDomain* domain = host->get_domain();
        for (int32_t i = 0; i < count; i++) {
            auto name = register_object->invoke(getnamespacedname, &i)->unbox<namespaced_name>();
            auto string_object = ref<managed::object>::create((MonoObject*)name.namespace_name, domain);
            std::string namespace_name = string_object->get_string();
            string_object = ref<managed::object>::create((MonoObject*)name.local_name, domain);
            std::string local_name = string_object->get_string();
            register_names.insert({ (size_t)i, namespace_name + ":" + local_name });
        }
        std::vector<std::pair<size_t, texture_data>> textures;
        for (const auto& [id, name] : register_names) {
            // For now, we only have a single texture per block
            auto path = this->get_asset_path("block:" + name + ".png");
            texture_data data;
            if (!graphics::texture::load_image(path, data.data, data.width, data.height, data.channels)) {
                continue;
            }
            textures.push_back({ id, data });
        }
        return ref<texture_atlas>(new texture_atlas(textures, object_factory));
    }
}
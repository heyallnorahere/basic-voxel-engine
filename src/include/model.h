#pragma once
#include "graphics/object_factory.h"
namespace bve {
    class model : public ref_counted {
    public:
        struct vertex {
            glm::vec3 position, normal;
            glm::vec2 uv;
            glm::vec3 tangent, bitangent;
        };
        struct mesh_data {
            size_t vertex_offset, index_offset;
            std::unordered_map<std::string, size_t> texture_offsets;
            std::vector<ref<graphics::texture>> textures;
        };
        model(const fs::path& path, ref<graphics::object_factory> object_factory);
        std::vector<vertex> get_vertices();
        std::vector<uint32_t> get_indices();
        size_t get_mesh_count();
        mesh_data get_mesh_data(size_t index);
    private:
        struct loaded_texture {
            ref<graphics::texture> texture;
            fs::path path;
        };
        void load();
        void process_node(aiNode* node);
        void process_mesh(aiMesh* mesh);
        void insert_textures(aiMesh* mesh, aiTextureType type, const std::string& type_name);
        std::vector<ref<graphics::texture>> load_material_textures(aiMaterial* material, aiTextureType type);
        const aiScene* m_scene;
        std::unique_ptr<Assimp::Importer> m_importer;
        fs::path m_path;
        std::vector<vertex> m_vertices;
        std::vector<uint32_t> m_indices;
        std::unordered_map<aiMesh*, std::pair<aiNode*, mesh_data>> m_mesh_data;
        std::vector<aiMesh*> m_meshes;
        std::vector<loaded_texture> m_loaded_textures;
        ref<graphics::object_factory> m_object_factory;
    };
}
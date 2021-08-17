#include "bve_pch.h"
#include "model.h"
namespace bve {
    template<glm::length_t L, typename T> static glm::vec<L, float> convert(const T& assimp_vector) {
        glm::vec<L, float> vector;
        for (size_t i = 0; i < (size_t)L; i++) {
            vector[i] = assimp_vector[i];
        }
        return vector;
    }
    model::model(const std::filesystem::path& path, ref<graphics::object_factory> object_factory) {
        this->m_path = path;
        this->m_importer = std::make_unique<Assimp::Importer>();
        this->m_object_factory = object_factory;
        this->load();
    }
    std::vector<model::vertex> model::get_vertices() {
        return this->m_vertices;
    }
    std::vector<uint32_t> model::get_indices() {
        return this->m_indices;
    }
    size_t model::get_mesh_count() {
        return this->m_meshes.size();
    }
    model::mesh_data model::get_mesh_data(size_t index) {
        aiMesh* mesh = this->m_meshes[index];
        return this->m_mesh_data[mesh].second;
    }
    void model::load() {
        constexpr uint32_t load_flags = 
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_GenUVCoords |
            aiProcess_LimitBoneWeights |
            aiProcess_FlipUVs;
        this->m_scene = this->m_importer->ReadFile(this->m_path.string(), load_flags);
        if (!this->m_scene || this->m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !this->m_scene->mRootNode) {
            spdlog::error("[model] assimp error: {}", this->m_importer->GetErrorString());
        } else {
            this->process_node(this->m_scene->mRootNode);
        }
    }
    void model::process_node(aiNode* node) {
        for (size_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = this->m_scene->mMeshes[node->mMeshes[i]];
            this->m_meshes.push_back(mesh);
            this->m_mesh_data[mesh].first = node;
            this->process_mesh(mesh);
        }
        for (size_t i = 0; i < node->mNumChildren; i++) {
            aiNode* child = node->mChildren[i];
            this->process_node(child);
        }
    }
    void model::process_mesh(aiMesh* mesh) {
        auto& mesh_data_ = this->m_mesh_data[mesh].second;
        mesh_data_.vertex_offset = this->m_vertices.size();
        mesh_data_.index_offset = this->m_indices.size();
        std::vector<vertex> vertices;
        std::vector<uint32_t> indices;
        for (size_t i = 0; i < mesh->mNumVertices; i++) {
            vertex v;
            v.position = convert<3>(mesh->mVertices[i]);
            v.normal = convert<3>(mesh->mNormals[i]);
            if (mesh->mTextureCoords[0]) {
                v.uv = convert<2>(mesh->mTextureCoords[0][i]);
            } else {
                v.uv = glm::vec2(0.f);
            }
            vertices.push_back(v);
        }
        for (size_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            if (face.mNumIndices != 3) {
                throw std::runtime_error("[model] face does not have exactly 3 indices!");
            }
            for (size_t j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        if (mesh->mMaterialIndex >= 0) {
            this->insert_textures(mesh, aiTextureType_AMBIENT, "ambient");
            this->insert_textures(mesh, aiTextureType_DIFFUSE, "diffuse");
            this->insert_textures(mesh, aiTextureType_SPECULAR, "specular");
            this->insert_textures(mesh, aiTextureType_NORMALS, "normal");
        }
        this->m_vertices.insert(this->m_vertices.end(), vertices.begin(), vertices.end());
        this->m_indices.insert(this->m_indices.end(), indices.begin(), indices.end());
    }
    void model::insert_textures(aiMesh* mesh, aiTextureType type, const std::string& type_name) {
        aiMaterial* material = this->m_scene->mMaterials[mesh->mMaterialIndex];
        auto& mesh_data_ = this->m_mesh_data[mesh].second;
        mesh_data_.texture_offsets[type_name] = mesh_data_.textures.size();
        auto textures = this->load_material_textures(material, type);
        mesh_data_.textures.insert(mesh_data_.textures.end(), textures.begin(), textures.end());
    }
    std::vector<ref<graphics::texture>> model::load_material_textures(aiMaterial* material, aiTextureType type) {
        std::vector<ref<graphics::texture>> textures;
        for (size_t i = 0; i < material->GetTextureCount(type); i++) {
            aiString path_;
            material->GetTexture(type, i, &path_);
            std::filesystem::path path = path_.C_Str();
            bool found = false;
            for (const auto& loaded_texture_ : this->m_loaded_textures) {
                if (loaded_texture_.path == path) {
                    textures.push_back(loaded_texture_.texture);
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::filesystem::path texture_path;
                if (path.is_absolute()) {
                    texture_path = path;
                } else {
                    texture_path = this->m_path.parent_path() / path;
                }
                ref<graphics::texture> texture = this->m_object_factory->create_texture(texture_path);
                textures.push_back(texture);
                loaded_texture loaded_texture_;
                loaded_texture_.texture = texture;
                loaded_texture_.path = path; // for the the comparison earlier
                this->m_loaded_textures.push_back(loaded_texture_);
            }
        }
        return textures;
    }
}
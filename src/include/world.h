#pragma once
namespace bve {
    template<glm::length_t L, typename T> struct hash_vector {
        using vector = glm::vec<L, T>;
        size_t operator()(const vector& key) const {
            std::hash<T> hasher;
            size_t hash = 0;
            for (glm::length_t i = 0; i < L; i++) {
                hash ^= hasher(key[i]) << (size_t)i;
            }
            return hash;
        }
    };
    class world {
    public:
        world(glm::ivec3 size);
    private:
        glm::ivec3 m_size;
        std::unordered_map<glm::ivec3, uint8_t, hash_vector<3, int32_t>> m_voxel_types;
    };
}
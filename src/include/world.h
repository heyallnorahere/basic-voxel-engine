#pragma once
#include "registry.h"
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
    class world;
    class entity {
    public:
        entity() = default;
        entity(const entity& other) = default;
        entity& operator=(const entity& other) = default;
        template<typename T, typename... Args> T& add_component(Args&&... args);
        template<typename T> T& get_component();
        template<typename T> void remove_component();
        template<typename T> bool has_component();
    private:
        entity(entt::entity handle, world* world_);
        entt::entity m_handle = entt::null;
        world* m_world = nullptr;
        friend class world;
    };
    class world : public ref_counted {
    public:
        using on_block_changed_callback = std::function<void(glm::ivec3, ref<world>)>;
        world();
        world(const world&) = delete;
        world& operator=(const world&) = delete;
        void update();
        entity create();
        void on_block_changed(on_block_changed_callback callback);
        void get_block(glm::ivec3 position, namespaced_name& block_type);
        void get_block(glm::ivec3 position, size_t& block_type);
        void set_block(glm::ivec3 position, const namespaced_name& block_type);
        void set_block(glm::ivec3 position, size_t block_type);
        std::vector<entity> get_cameras();
        std::vector<glm::ivec3> get_set_blocks();
    private:
        template<typename T> void on_component_added(T& component, entity ent);
        std::unordered_map<glm::ivec3, uint8_t, hash_vector<3, int32_t>> m_voxel_types;
        entt::registry m_registry;
        std::vector<on_block_changed_callback> m_on_block_changed;
        friend class entity;
    };
    template<typename T, typename... Args> inline T& entity::add_component(Args&&... args) {
        if (this->has_component<T>()) {
            throw std::runtime_error("[entity] a component of the specified type already exists on this entity!");
        }
        T& component = this->m_world->m_registry.emplace<T>(this->m_handle, std::forward<Args>(args)...);
        this->m_world->on_component_added(component, *this);
        return component;
    }
    template<typename T> inline T& entity::get_component() {
        if (!this->has_component<T>()) {
            throw std::runtime_error("[entity] a component of the specified type does not exist on this entity!");
        }
        return this->m_world->m_registry.get<T>(this->m_handle);
    }
    template<typename T> inline void entity::remove_component() {
        if (!this->has_component<T>()) {
            throw std::runtime_error("[entity] a component of the specified type does not exist on this entity!");
        }
        this->m_world->m_registry.remove<T>(this->m_handle);
    }
    template<typename T> inline bool entity::has_component() {
        return this->m_world->m_registry.all_of<T>(this->m_handle);
    }
    template<typename T> inline void world::on_component_added(T& component, entity ent) {
        // no specific implementation so let's just return
    }
}
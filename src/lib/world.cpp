#include "bve_pch.h"
#include "world.h"
#include "registry.h"
namespace bve {
    entity::entity(const entity& other) {
        this->m_handle = other.m_handle;
        this->m_world = other.m_world;
    }
    entity& entity::operator=(const entity& other) {
        this->m_handle = other.m_handle;
        this->m_world = other.m_world;
        return *this;
    }
    entity::entity(entt::entity handle, world* world_) {
        this->m_handle = handle;
        this->m_world = world_;
    }
    world::world(glm::ivec3 size) {
        this->m_size = size;
        for (int32_t x = 0; x < this->m_size.x; x++) {
            for (int32_t y = 0; y < this->m_size.y; y++) {
                for (int32_t z = 0; z < this->m_size.z; z++) {
                    this->m_voxel_types[glm::ivec3(x, y, z)] = 0;
                }
            }
        }
    }
    void world::update() {
        // todo: update
    }
    entity world::create() {
        entity ent = entity(this->m_registry.create(), this);
        // todo: add transform component
        return ent;
    }
}
#include "bve_pch.h"
#include "world.h"
#include "block.h"
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
        // temporary
        this->m_voxel_types[glm::ivec3(0)] = 1;
        this->m_voxel_types[glm::ivec3(1, 1, 0)] = 1;
        this->m_voxel_types[glm::ivec3(1, 0, 0)] = 1;
    }
    void world::update() {
        // todo: update
    }
    entity world::create() {
        entity ent = entity(this->m_registry.create(), this);
        // todo: add transform component
        return ent;
    }
    glm::ivec3 world::get_size() {
        return this->m_size;
    }
    void world::get_block(glm::ivec3 position, namespaced_name& block_type) {
        if (this->m_voxel_types.find(position) == this->m_voxel_types.end()) {
            throw std::runtime_error("[world] could not find a block at the specified position!");
        }
        auto& block_register = registry::get().get_register<block>();
        uint8_t index = this->m_voxel_types[position];
        auto name = block_register.get_name((size_t)index);
        if (name) {
            block_type = *name;
        } else {
            throw std::runtime_error("[world] could not find the namespaced id of the specified block");
        }
    }
    void world::get_block(glm::ivec3 position, size_t& block_type) {
        if (this->m_voxel_types.find(position) == this->m_voxel_types.end()) {
            throw std::runtime_error("[world] could not find a block at the specified position!");
        }
        auto& block_register = registry::get().get_register<block>();
        uint8_t index = this->m_voxel_types[position];
        block_type = (size_t)index;        
    }
}
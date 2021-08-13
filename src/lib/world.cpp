#include "bve_pch.h"
#include "world.h"
#include "block.h"
#include "components.h"
namespace bve {
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
        auto& block_register = registry::get().get_register<block>();
        uint8_t test_block = (uint8_t)*block_register.get_index("bve:test_block");
        uint8_t test_block_2 = (uint8_t)*block_register.get_index("bve:test_block_2");
        this->m_voxel_types[glm::ivec3(0)] = test_block;
        this->m_voxel_types[glm::ivec3(1, 2, 0)] = test_block;
        this->m_voxel_types[glm::ivec3(1, 0, 0)] = test_block_2;
    }
    void world::update() {
        // todo: update
    }
    entity world::create() {
        entity ent = entity(this->m_registry.create(), this);
        ent.add_component<components::transform_component>();
        return ent;
    }
    glm::ivec3 world::get_size() {
        return this->m_size;
    }
    void world::on_block_changed(on_block_changed_callback callback) {
        this->m_on_block_changed.push_back(callback);
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
    void world::set_block(glm::ivec3 position, const namespaced_name& block_type) {
        auto& block_register = registry::get().get_register<block>();
        std::optional<size_t> index = block_register.get_index(block_type);
        if (!index) {
            throw std::runtime_error("[world] attempted to access a block that did not exist");
        }
        this->set_block(position, *index);
    }
    void world::set_block(glm::ivec3 position, size_t block_type) {
        auto& block_register = registry::get().get_register<block>();
        if (block_type >= block_register.size()) {
            throw std::runtime_error("[world] attempted to access an unregistered block");
        }
        this->m_voxel_types[position] = (uint8_t)block_type;
        for (const auto& callback : this->m_on_block_changed) {
            callback(position, ref<world>(this));
        }
    }
}
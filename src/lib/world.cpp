#include "bve_pch.h"
#include "world.h"
#include "block.h"
#include "components.h"
#include "code_host.h"
namespace bve {
    entity::entity(entt::entity handle, world* world_) {
        this->m_handle = handle;
        this->m_world = world_;
    }
    void world::generate() {
        ref<code_host> host = code_host::current();
        ref<managed::class_> world_class = host->find_class("BasicVoxelEngine.World");
        ref<world>* pointer = new ref<world>(this);
        ref<managed::object> world_instance = world_class->instantiate(&pointer);
        ref<managed::class_> builder_class = host->find_class("BasicVoxelEngine.WorldGen.Builder");
        auto generate = builder_class->get_method("*:Generate");
        int32_t seed = -1;
        builder_class->invoke(generate, world_instance->get(), &seed);
    }
    void world::update() {
        auto script_view = this->m_registry.view<components::script_component>();
        script_view.each([](components::script_component& sc) {
            for (auto& script : sc.scripts) {
                script.update();
            }
        });
    }
    entity world::create() {
        entity ent = entity(this->m_registry.create(), this);
        ent.add_component<components::transform_component>();
        return ent;
    }
    void world::on_block_changed(on_block_changed_callback callback) {
        this->m_on_block_changed.push_back(callback);
    }
    void world::get_block(glm::ivec3 position, namespaced_name& block_type) {
        auto& block_register = registry::get().get_register<block>();
        size_t index;
        this->get_block(position, index);
        auto name = block_register.get_name(index);
        if (name) {
            block_type = *name;
        } else {
            throw std::runtime_error("[world] could not find the namespaced id of the specified block");
        }
    }
    void world::get_block(glm::ivec3 position, size_t& block_type) {
        if (this->m_voxel_types.find(position) == this->m_voxel_types.end()) {
            block_type = 0;
            return;
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
        if (block_type == 0) {
            this->m_voxel_types.erase(position);
        } else {
            this->m_voxel_types[position] = (uint8_t)block_type;
        }
        for (const auto& callback : this->m_on_block_changed) {
            callback(position, ref<world>(this));
        }
    }
    std::vector<entity> world::get_cameras() {
        auto camera_view = this->m_registry.view<components::camera_component>();
        std::vector<entity> entities;
        for (entt::entity ent : camera_view) {
            entities.push_back(entity(ent, this));
        }
        return entities;
    }
    std::vector<glm::ivec3> world::get_set_blocks() {
        std::vector<glm::ivec3> positions;
        for (const auto& pair : this->m_voxel_types) {
            positions.push_back(pair.first);
        }
        return positions;
    }
}
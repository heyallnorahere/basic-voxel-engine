#include "bve_pch.h"
#include "world.h"
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
        managed::class_::invoke(generate, world_instance->get(), &seed);
    }
    void world::update() {
        // todo: update blocks
    }
    entity world::create() {
        entity ent = entity(this->m_registry.create(), this);
        ent.add_component<components::transform_component>();
        return ent;
    }
    void world::on_block_changed(on_block_changed_callback callback) {
        this->m_on_block_changed.push_back(callback);
    }
    void world::get_block(glm::ivec3 position, size_t& block_type) {
        if (this->m_voxel_types.find(position) == this->m_voxel_types.end()) {
            block_type = 0;
            return;
        }
        uint8_t index = this->m_voxel_types[position];
        block_type = (size_t)index;        
    }
    void world::set_block(glm::ivec3 position, size_t block_type) {
        auto host = code_host::current();
        auto helpers_class = host->find_class("BasicVoxelEngine.Helpers");
        auto getregister = helpers_class->get_method("*:GetRegister");
        auto block_class = host->find_class("BasicVoxelEngine.Block");
        auto type_object = managed::type::get_type(block_class)->get_object();
        auto register_object = managed::class_::invoke(getregister, type_object);
        auto register_class = managed::class_::get_class(register_object);
        auto count_property = register_class->get_property("Count");
        int32_t count = register_object->get(count_property)->unbox<int32_t>();
        if (block_type >= (size_t)count) {
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
    std::vector<entity> world::get_scripted_entities() {
        auto script_view = this->m_registry.view<components::script_component>();
        std::vector<entity> entities;
        for (entt::entity ent : script_view) {
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
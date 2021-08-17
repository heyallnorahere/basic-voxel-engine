#include "bve_pch.h"
#include "block.h"
#include "lighting/spotlight.h"
#include "asset_manager.h"
namespace bve {

    // blocks
    namespace blocks {
        class air : public block {
        public:
            virtual float opacity() override { return 0.f; }
            virtual bool solid() override { return false; }
            virtual std::string friendly_name() override { return "Air"; }
        };
        class test_block : public block {
        public:
            virtual std::string friendly_name() override { return "Test Block 1"; }
        };
        class test_block_2 : public block {
        public:
            virtual void load(ref<graphics::object_factory> object_factory, const namespaced_name& register_name) override {
                auto light = ref<lighting::spotlight>::create();
                light->set_ambient_strength(0.01f);
                light->set_specular_strength(0.5f);
                light->set_color(glm::vec3(1.f));
                light->set_cutoff(cos(glm::radians(30.f)));
                light->set_direction(glm::vec3(0.f, -1.f, 0.f));
                this->m_light = light;
            }
            virtual std::string friendly_name() override { return "Test Block 2"; }
            virtual ref<lighting::light> get_light() override { return this->m_light; }
        private:
            ref<lighting::light> m_light;
        };
        class model_block : public block {
        public:
            virtual void load(ref<graphics::object_factory> object_factory, const namespaced_name& register_name) override {
                std::string asset_name = "model:" + register_name.get_full_name() + ".obj";
                auto path = asset_manager::get().get_asset_path(asset_name);
                this->m_model = ref<model>::create(path, object_factory);
            }
            virtual std::string friendly_name() override { return "Model block"; }
            virtual ref<model> get_model() override { return this->m_model; }
        private:
            ref<model> m_model;
        };
    }

    block::~block() { }
    void block::load(ref<graphics::object_factory> object_factory, const namespaced_name& register_name) { }
    float block::opacity() { return 1.f; }
    bool block::solid() { return true; }
    ref<lighting::light> block::get_light() { return nullptr; }
    ref<model> block::get_model() { return nullptr; }
    void block::register_all() {
        auto& block_register = registry::get().get_register<block>();
#define register(block_type) block_register.add(ref<blocks::block_type>::create(), "bve:" + std::string(#block_type))
        register(air);
        register(test_block);
        register(test_block_2);
        register(model_block);
#undef register
    }
}
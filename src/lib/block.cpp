#include "bve_pch.h"
#include "block.h"
#include "registry.h"
#include "lighting/point_light.h"
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
            test_block_2() {
                this->m_light = ref<lighting::point_light>::create();
                this->m_light->set_ambient_strength(0.1f);
                this->m_light->set_specular_strength(0.5f);
                this->m_light->set_color(glm::vec3(1.f));
                this->m_light->set_constant(1.f);
                this->m_light->set_linear(0.09f);
                this->m_light->set_quadratic(0.032f);
            }
            virtual std::string friendly_name() override { return "Test Block 2"; }
            virtual ref<lighting::light> get_light() override { return this->m_light; }
        private:
            ref<lighting::point_light> m_light;
        };
    }

    block::~block() { }
    float block::opacity() { return 1.f; }
    bool block::solid() { return true; }
    ref<lighting::light> block::get_light() { return nullptr; }
    void block::register_all() {
        auto& block_register = registry::get().get_register<block>();
#define register(block_type) block_register.add(ref<blocks::block_type>::create(), "bve:" + std::string(#block_type))
        register(air);
        register(test_block);
        register(test_block_2);
#undef register
    }
}
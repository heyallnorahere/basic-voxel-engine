#include "bve_pch.h"
#include "block.h"
#include "registry.h"
namespace bve {

    // blocks
    namespace blocks {
        class air : public block {
        public:
            virtual float opacity() override { return 0.f; }
            virtual bool solid() override { return false; }
        };
        class test_block : public block {
        public:
            // todo: override stuff i guess
        };
    }

    block::~block() { }
    float block::opacity() { return 1.f; }
    bool block::solid() { return true; }
    void block::register_all() {
        auto& block_register = registry::get().get_register<block>();
#define register(block_type) block_register.add(std::shared_ptr<block>(new blocks::block_type), "bve:" + std::string(#block_type))
        register(air);
        register(test_block);
#undef register
    }
}
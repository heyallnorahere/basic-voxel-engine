#include "bve_pch.h"
#include "block.h"
#include "lighting/spotlight.h"
#include "asset_manager.h"
namespace bve {
    block::~block() { }
    void block::load(ref<graphics::object_factory> object_factory, const namespaced_name& register_name) { }
    float block::opacity() { return 1.f; }
    bool block::solid() { return true; }
    ref<lighting::light> block::get_light() { return nullptr; }
    ref<model> block::get_model() { return nullptr; }
    bool block::managed() { return false; }
}
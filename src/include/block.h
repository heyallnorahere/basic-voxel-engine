#pragma once
#include "graphics/object_factory.h"
#include "lighting/light.h"
#include "model.h"
#include "registry.h"
namespace bve {
    class block : public ref_counted {
    public:
        virtual ~block();
        virtual void load(ref<graphics::object_factory> object_factory, const namespaced_name& register_name);
        virtual float opacity();
        virtual bool solid();
        virtual ref<lighting::light> get_light();
        virtual ref<model> get_model();
        virtual std::string friendly_name() = 0;
        virtual bool managed();
        // todo: add more properties
    };
}
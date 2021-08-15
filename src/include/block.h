#pragma once
#include "lighting/light.h"
namespace bve {
    class block : public ref_counted {
    public:
        virtual ~block();
        virtual float opacity();
        virtual bool solid();
        virtual ref<lighting::light> get_light();
        virtual std::string friendly_name() = 0;
        // todo: add more properties
        static void register_all();
    };
}
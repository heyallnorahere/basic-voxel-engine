#pragma once
namespace bve {
    class block {
    public:
        virtual ~block();
        virtual float opacity();
        virtual bool solid();
        // todo: add more properties
        static void register_all();
    };
}
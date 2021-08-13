#pragma once
namespace bve {
    class block : public ref_counted {
    public:
        virtual ~block();
        virtual float opacity();
        virtual bool solid();
        virtual std::string friendly_name() = 0;
        // todo: add more properties
        static void register_all();
    };
}
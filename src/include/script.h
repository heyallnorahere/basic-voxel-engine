#pragma once
#include "world.h"
namespace bve {
    namespace components {
        struct script_component;
    }
    class script : public ref_counted {
    public:
        script() = default;
        virtual ~script() { }
        script(const script&) = delete;
        script& operator=(const script&) = delete;
        virtual void on_attach() { }
        virtual void update() = 0;
    protected:
        template<typename T, typename... Args> T& add_component(Args&&... args) {
            return this->m_entity.add_component<T>(std::forward<Args>(args)...);
        }
        template<typename T> T& get_component() {
            return this->m_entity.get_component<T>();
        }
        template<typename T> void remove_component() {
            this->m_entity.remove_component<T>();
        }
        template<typename T> bool has_component() {
            return this->m_entity.has_component<T>();
        }
        entity get_entity() {
            return this->m_entity;
        }
    private:
        entity m_entity;
        friend struct components::script_component;
    };
}